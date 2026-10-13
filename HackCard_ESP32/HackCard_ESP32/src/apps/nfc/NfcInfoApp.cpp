#include "NfcInfoApp.h"

#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"

NfcInfoApp NfcInfo;

bool NfcInfoApp::ensureReady() {
  if (Nfc.isReady()) {
    return true;
  }
  return Nfc.begin();
}

NfcInfoResult NfcInfoApp::readInfo(uint32_t timeoutMs) {
  NfcInfoResult result;

  if (!ensureReady()) {
    strncpy(result.tag.error, "PN532 not detected on I2C bus", sizeof(result.tag.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.readTagDetail(result.tag, timeoutMs)) {
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::success();

  String summary = String(result.tag.uidHex) + " " + result.tag.family;
  Log.activity("nfc", "info", summary.c_str());
  NfcHist.append("info", result.tag.uidHex, result.tag.family);

  String line = String(result.tag.uidHex) + "," + result.tag.family + "," + result.tag.ndefPreview + "\n";
  Storage.writeTextFile("/nfc/last_info.txt", line);

  return result;
}

String NfcInfoApp::toJson(const NfcInfoResult& result) const {
  const NfcTagDetail& t = result.tag;
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"chip_ready\":";
  json += Nfc.isReady() ? "true" : "false";
  json += ",\"uid\":\"";
  json += t.uidHex;
  json += "\",\"uid_length\":";
  json += String(t.uidLength);
  json += ",\"tag_type\":\"";
  json += t.tagType;
  json += "\",\"family\":\"";
  json += t.family;
  json += "\",\"memory_hint\":\"";
  json += t.memoryHint;
  json += "\",\"ndef_preview\":\"";
  json += t.ndefPreview;
  json += "\",\"error\":\"";
  json += t.error;
  json += "\"}";
  return json;
}

String NfcInfoApp::toText(const NfcInfoResult& result) const {
  String text = "── NFC Tag Info ──\n";
  text += "Chip: ";
  text += Nfc.getChipInfo();
  text += "\n";

  if (!result.success) {
    text += "Error: ";
    text += result.tag.error;
    text += "\n";
    return text;
  }

  const NfcTagDetail& t = result.tag;
  text += "UID:     ";
  text += t.uidHex;
  text += "\nType:    ";
  text += t.tagType;
  text += "\nFamily:  ";
  text += t.family;
  text += "\nMemory:  ";
  text += t.memoryHint;
  text += "\n";
  if (t.ndefPreview[0] != '\0') {
    text += "NDEF:    ";
    text += t.ndefPreview;
    text += "\n";
  }
  return text;
}
