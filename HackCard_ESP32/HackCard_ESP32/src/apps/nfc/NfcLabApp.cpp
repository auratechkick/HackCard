#include "NfcLabApp.h"

#include "NfcDumpParser.h"
#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"

NfcCloneApp NfcClone;
NfcKillApp NfcKill;

bool NfcCloneApp::ensureReady() {
  return Nfc.isReady() || Nfc.begin();
}

NfcCloneResult NfcCloneApp::restoreDump(const char* dumpPath, const char* labPin, uint8_t startPage) {
  NfcCloneResult result;

  if (!nfcLabPinOk(labPin)) {
    strncpy(result.error, "Lab PIN required — nfc clone <pin>", sizeof(result.error) - 1);
    return result;
  }

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  const char* path = dumpPath;
  String lastPath;
  if (!path || path[0] == '\0' || strcmp(path, "last") == 0) {
    lastPath = NfcHist.lastDumpPath();
    path = lastPath.c_str();
  }

  if (!path || path[0] == '\0') {
    strncpy(result.error, "No dump file — run nfc dump first", sizeof(result.error) - 1);
    return result;
  }

  strlcpy(result.sourcePath, path, sizeof(result.sourcePath));

  NfcDumpData dump;
  if (!NfcDumpParser::loadDumpPath(path, dump, result.error, sizeof(result.error))) {
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.restoreType2Pages(dump, startPage, result.error, sizeof(result.error))) {
    NfcFeedback::error();
    return result;
  }

  NfcTagInfo tag;
  if (Nfc.readTag(tag, 800)) {
    strlcpy(result.destUid, tag.uidHex, sizeof(result.destUid));
  }

  result.pagesWritten = dump.pageCount - startPage;
  result.success = true;
  NfcFeedback::cloneOk();
  Log.activity("nfc", "clone", result.destUid);
  NfcHist.append("clone", result.destUid, result.sourcePath);
  return result;
}

NfcCloneResult NfcCloneApp::restoreLast(const char* labPin) {
  return restoreDump("last", labPin, 4);
}

String NfcCloneApp::toJson(const NfcCloneResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"source\":\"";
  json += result.sourcePath;
  json += "\",\"dest_uid\":\"";
  json += result.destUid;
  json += "\",\"pages_written\":";
  json += String(result.pagesWritten);
  json += ",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}

String NfcCloneApp::toText(const NfcCloneResult& result) const {
  if (!result.success) {
    return String("── NFC Clone FAILED ──\n") + result.error + "\n";
  }
  String text = "── NFC Clone OK ──\n";
  text += "Source: " + String(result.sourcePath) + "\n";
  text += "Dest:   " + String(result.destUid) + "\n";
  text += "Pages:  " + String(result.pagesWritten) + " (from page 4)\n";
  text += "Note:   UID stays unique on destination tag\n";
  return text;
}

bool NfcKillApp::ensureReady() {
  return Nfc.isReady() || Nfc.begin();
}

NfcKillResult NfcKillApp::killTag(const char* labPin) {
  NfcKillResult result;

  if (!nfcLabPinOk(labPin)) {
    strncpy(result.error, "Lab PIN required — nfc kill <pin>", sizeof(result.error) - 1);
    return result;
  }

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.killType2Tag(result.error, sizeof(result.error))) {
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::killOk();
  Log.activity("nfc", "kill", "locked");
  NfcHist.append("kill", "tag", "permanent-lock");
  return result;
}

String NfcKillApp::toJson(const NfcKillResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  if (result.success) {
    json += ",\"message\":\"Tag permanently locked — no further writes\"";
  }
  json += ",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}

String NfcKillApp::toText(const NfcKillResult& result) const {
  if (!result.success) {
    return String("── NFC Kill FAILED ──\n") + result.error + "\n";
  }
  return String("── NFC Kill OK ──\nTag permanently locked. User data cleared.\n");
}
