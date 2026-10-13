#include "NfcDumpApp.h"

#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"

NfcDumpApp NfcDump;

bool NfcDumpApp::ensureReady() {
  if (Nfc.isReady()) {
    return true;
  }
  return Nfc.begin();
}

String NfcDumpApp::formatDumpFile(const NfcDumpResult& result) const {
  String text = "# HackCard NFC Dump\n";
  text += "uid=" + String(result.dump.tag.uidHex) + "\n";
  text += "type=" + String(result.dump.tag.tagType) + "\n";
  text += "pages=" + String(result.dump.pageCount) + "\n\n";

  for (uint8_t i = 0; i < result.dump.pageCount; i++) {
    text += "page" + String(i) + "=";
    for (uint8_t b = 0; b < 4; b++) {
      if (b > 0) {
        text += " ";
      }
      char hex[4];
      snprintf(hex, sizeof(hex), "%02X", result.dump.pages[i][b]);
      text += hex;
    }
    text += "\n";
  }
  return text;
}

bool NfcDumpApp::saveDump(NfcDumpResult& result) {
  char path[56];
  snprintf(path, sizeof(path), "/nfc/dumps/%lu_%s.txt", millis(), result.dump.tag.uidHex);
  for (char* p = path; *p; p++) {
    if (*p == ':') {
      *p = '_';
    }
  }

  const String body = formatDumpFile(result);
  bool ok = false;
  if (NfcHist.dumpsSdOnly()) {
    if (!Storage.isSdReady()) {
      return false;
    }
    Storage.ensureDirectory("/nfc/exports/dumps");
    char sdPath[64];
    snprintf(sdPath, sizeof(sdPath), "/nfc/exports/dumps/%lu_%s.txt", millis(), result.dump.tag.uidHex);
    for (char* p = sdPath; *p; p++) {
      if (*p == ':') {
        *p = '_';
      }
    }
    ok = Storage.writeTextFileSdOnly(sdPath, body);
    if (ok) {
      strlcpy(path, sdPath, sizeof(path));
    }
  } else {
    ok = Storage.writeTextFile(path, body);
  }
  if (!ok) {
    return false;
  }
  result.savedPath = path;
  NfcHist.setLastDumpPath(path);
  return true;
}

NfcDumpResult NfcDumpApp::dumpOnce(uint8_t maxPages) {
  NfcDumpResult result;

  if (!ensureReady()) {
    strncpy(result.dump.error, "PN532 not ready", sizeof(result.dump.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.dumpType2Tag(result.dump, maxPages)) {
    NfcFeedback::error();
    return result;
  }

  result.preview = formatDumpFile(result);
  if (result.preview.length() > 512) {
    result.preview = result.preview.substring(0, 512) + "\n...(truncated)";
  }

  if (!saveDump(result)) {
    strncpy(result.dump.error, "Could not save dump file", sizeof(result.dump.error) - 1);
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::success();
  Log.activity("nfc", "dump", result.dump.tag.uidHex);
  NfcHist.append("dump", result.dump.tag.uidHex, result.savedPath.c_str());
  return result;
}

String NfcDumpApp::toJson(const NfcDumpResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"uid\":\"";
  json += result.dump.tag.uidHex;
  json += "\",\"tag_type\":\"";
  json += result.dump.tag.tagType;
  json += "\",\"pages\":";
  json += String(result.dump.pageCount);
  json += ",\"saved_path\":\"";
  json += result.savedPath;
  json += "\",\"error\":\"";
  json += result.dump.error;
  json += "\"}";
  return json;
}

String NfcDumpApp::toText(const NfcDumpResult& result) const {
  if (!result.success) {
    String text = "── NFC Dump FAILED ──\n";
    text += result.dump.error;
    text += "\n";
    return text;
  }
  String text = "── NFC Dump OK ──\n";
  text += "UID:   " + String(result.dump.tag.uidHex) + "\n";
  text += "Pages: " + String(result.dump.pageCount) + "\n";
  text += "Saved: " + result.savedPath + "\n";
  return text;
}
