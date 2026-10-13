#include "NfcReadApp.h"

#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"

NfcReadApp NfcRead;

bool NfcReadApp::ensureReady() {
  if (Nfc.isReady()) {
    return true;
  }
  return Nfc.begin();
}

NfcReadResult NfcReadApp::readOnce(uint32_t timeoutMs) {
  NfcReadResult result;

  if (!ensureReady()) {
    strncpy(result.tag.error, "PN532 not detected on I2C bus", sizeof(result.tag.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.readTag(result.tag, timeoutMs)) {
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  saveRead(result.tag, result.savedPath);
  NfcFeedback::success();
  if (result.tag.uidHex[0] != '\0') {
    Log.activity("nfc", "read", result.tag.uidHex);
    NfcHist.append("read", result.tag.uidHex, result.savedPath.c_str());
  }
  return result;
}

bool NfcReadApp::saveRead(const NfcTagInfo& tag, String& savedPath) {
  if (!tag.detected) {
    return false;
  }

  String line = String(tag.uidHex) + "," + tag.tagType + "," + String(millis()) + "\n";
  String dump = "uid=";
  dump += tag.uidHex;
  dump += "\ntype=";
  dump += tag.tagType;
  dump += "\n";

  if (NfcHist.dumpsSdOnly()) {
    if (!Storage.isSdReady()) {
      return false;
    }
    Storage.ensureDirectory("/nfc/exports/reads");
    char path[56];
    snprintf(path, sizeof(path), "/nfc/exports/reads/%lu_%s.txt", millis(), tag.uidHex);
    for (char* p = path; *p; p++) {
      if (*p == ':') {
        *p = '_';
      }
    }
    if (Storage.writeTextFileSdOnly(path, dump)) {
      savedPath = path;
      Storage.writeTextFile("/nfc/last_read.txt", line);
      return true;
    }
    return false;
  }

  Storage.writeTextFile("/nfc/last_read.txt", line);

  if (Storage.isSdReady()) {
    char path[48];
    snprintf(path, sizeof(path), "/nfc/dumps/%lu.txt", millis());
    if (Storage.writeTextFile(path, dump)) {
      savedPath = path;
      return true;
    }
  }

  savedPath = "/nfc/last_read.txt";
  return true;
}

String NfcReadApp::toJson(const NfcReadResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"chip_ready\":";
  json += Nfc.isReady() ? "true" : "false";
  json += ",\"chip_info\":\"";
  json += Nfc.getChipInfo();
  json += "\",\"uid\":\"";
  json += result.tag.uidHex;
  json += "\",\"uid_length\":";
  json += String(result.tag.uidLength);
  json += ",\"tag_type\":\"";
  json += result.tag.tagType;
  json += "\",\"error\":\"";
  json += result.tag.error;
  json += "\",\"saved_path\":\"";
  json += result.savedPath;
  json += "\"}";
  return json;
}

String NfcReadApp::toText(const NfcReadResult& result) const {
  String text = "── NFC Read ──\n";
  text += "Chip: ";
  text += Nfc.getChipInfo();
  text += "\n";

  if (result.success) {
    text += "UID:  ";
    text += result.tag.uidHex;
    text += "\nType: ";
    text += result.tag.tagType;
    text += "\nSaved: ";
    text += result.savedPath;
    text += "\n";
  } else {
    text += "Error: ";
    text += result.tag.error;
    text += "\n";
  }
  return text;
}
