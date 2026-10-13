#include "NfcClassicApp.h"

#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"

NfcClassicApp NfcClassic;

bool NfcClassicApp::ensureReady() {
  return Nfc.isReady() || Nfc.begin();
}

String NfcClassicApp::formatClassicFile(const NfcClassicResult& result) const {
  String text = "# HackCard Mifare Classic Dump\n";
  text += "uid=" + String(result.dump.tag.uidHex) + "\n";
  text += "key=" + String(result.dump.keyHex) + "\n";
  text += "sectors=" + String(result.dump.sectorCount) + "\n\n";

  for (uint8_t s = 0; s < result.dump.sectorCount; s++) {
    if (!result.dump.sectors[s].readable) {
      continue;
    }
    text += "sector" + String(s) + "=\n";
    for (uint8_t b = 0; b < 3; b++) {
      text += "block" + String(b) + "=";
      for (uint8_t i = 0; i < 16; i++) {
        if (i > 0) {
          text += " ";
        }
        char hex[4];
        snprintf(hex, sizeof(hex), "%02X", result.dump.sectors[s].blocks[b][i]);
        text += hex;
      }
      text += "\n";
    }
  }
  return text;
}

bool NfcClassicApp::saveClassicDump(NfcClassicResult& result) {
  char path[56];
  snprintf(path, sizeof(path), "/nfc/classic/%lu_%s.txt", millis(), result.dump.tag.uidHex);
  for (char* p = path; *p; p++) {
    if (*p == ':') {
      *p = '_';
    }
  }

  if (!Storage.exists("/nfc/classic")) {
    Storage.writeTextFile("/nfc/classic/.keep", "");
  }

  const String body = formatClassicFile(result);
  bool ok = false;
  if (NfcHist.dumpsSdOnly()) {
    if (!Storage.isSdReady()) {
      return false;
    }
    Storage.ensureDirectory("/nfc/exports/classic");
    char sdPath[64];
    snprintf(sdPath, sizeof(sdPath), "/nfc/exports/classic/%lu_%s.txt", millis(), result.dump.tag.uidHex);
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
  _lastSavedPath = path;
  return true;
}

NfcClassicResult NfcClassicApp::dumpWithKey(const char* keyHex, uint8_t maxSectors) {
  NfcClassicResult result;

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  uint8_t key[6] = {0};
  if (!Nfc.parseClassicKeyHex(keyHex ? keyHex : "FFFFFFFFFFFF", key, result.error, sizeof(result.error))) {
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.dumpClassicTag(key, result.dump, maxSectors)) {
    strncpy(result.error, result.dump.error, sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  if (!saveClassicDump(result)) {
    strncpy(result.error, "Could not save classic dump", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  _lastDump = result.dump;
  _hasLastDump = true;

  result.success = true;
  NfcFeedback::success();
  Log.activity("nfc", "classic_dump", result.dump.tag.uidHex);
  NfcHist.append("classic_dump", result.dump.tag.uidHex, result.savedPath.c_str());
  return result;
}

NfcClassicResult NfcClassicApp::cloneLast(const char* keyHex, const char* labPin) {
  NfcClassicResult result;

  if (!nfcLabPinOk(labPin)) {
    strncpy(result.error, "Lab PIN required", sizeof(result.error) - 1);
    return result;
  }

  if (!_hasLastDump) {
    strncpy(result.error, "Run nfc classic dump <key> first (same session)", sizeof(result.error) - 1);
    return result;
  }

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    return result;
  }

  const NfcClassicDump& dump = _lastDump;
  uint8_t key[6] = {0};
  const char* useKey = keyHex && keyHex[0] ? keyHex : dump.keyHex;
  if (!Nfc.parseClassicKeyHex(useKey, key, result.error, sizeof(result.error))) {
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.cloneClassicDump(dump, key, result.error, sizeof(result.error))) {
    NfcFeedback::error();
    return result;
  }

  result.dump = dump;
  result.success = true;
  NfcFeedback::cloneOk();
  Log.activity("nfc", "classic_clone", dump.tag.uidHex);
  NfcHist.append("classic_clone", "dest", dump.keyHex);
  return result;
}

NfcKeyScanResult NfcClassicApp::scanKey(uint8_t sector) {
  NfcKeyScanResult result;
  result.sector = sector;

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.scanClassicKey(sector, result.keyHex, sizeof(result.keyHex))) {
    strncpy(result.error, "No common key matched — try manual key", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::success();
  Log.activity("nfc", "keyscan", result.keyHex);
  NfcHist.append("keyscan", "sector", result.keyHex);
  return result;
}

String NfcClassicApp::dumpToJson(const NfcClassicResult& result) const {
  uint8_t readable = 0;
  for (uint8_t i = 0; i < result.dump.sectorCount; i++) {
    if (result.dump.sectors[i].readable) {
      readable++;
    }
  }

  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"uid\":\"";
  json += result.dump.tag.uidHex;
  json += "\",\"key\":\"";
  json += result.dump.keyHex;
  json += "\",\"sectors_readable\":";
  json += String(readable);
  json += ",\"saved_path\":\"";
  json += result.savedPath;
  json += "\",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}

String NfcClassicApp::dumpToText(const NfcClassicResult& result) const {
  if (!result.success) {
    return String("── Classic Dump FAILED ──\n") + result.error + "\n";
  }

  uint8_t readable = 0;
  for (uint8_t i = 0; i < result.dump.sectorCount; i++) {
    if (result.dump.sectors[i].readable) {
      readable++;
    }
  }

  String text = "── Classic Dump OK ──\n";
  text += "UID:     " + String(result.dump.tag.uidHex) + "\n";
  text += "Key:     " + String(result.dump.keyHex) + "\n";
  text += "Sectors: " + String(readable) + " readable\n";
  text += "Saved:   " + result.savedPath + "\n";
  return text;
}

String NfcClassicApp::keyScanToJson(const NfcKeyScanResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"sector\":";
  json += String(result.sector);
  json += ",\"key\":\"";
  json += result.keyHex;
  json += "\",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}

String NfcClassicApp::keyScanToText(const NfcKeyScanResult& result) const {
  if (!result.success) {
    return String("── Key Scan FAILED ──\n") + result.error + "\n";
  }
  String text = "── Key Scan OK ──\n";
  text += "Sector: " + String(result.sector) + "\n";
  text += "Key:    " + String(result.keyHex) + "\n";
  return text;
}
