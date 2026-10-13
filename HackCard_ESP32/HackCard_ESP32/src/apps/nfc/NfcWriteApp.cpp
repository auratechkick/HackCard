#include "NfcWriteApp.h"

#include "../demo/LedBuzzerApp.h"
#include "NfcHistory.h"
#include "NfcFeedback.h"
#include "../../core/Logger.h"
#include "../../platform/hal/HalNfc.h"

NfcWriteApp NfcWrite;

namespace {

bool writeUriAdapter(const char* payload, char* errorOut, size_t errorOutSize) {
  return Nfc.writeNdefUri(payload, errorOut, errorOutSize);
}

bool writeTextAdapter(const char* payload, char* errorOut, size_t errorOutSize) {
  return Nfc.writeNdefText(payload, errorOut, errorOutSize);
}

bool writeVcardAdapter(const char* payload, char* errorOut, size_t errorOutSize) {
  return Nfc.writeNdefVcard(payload, errorOut, errorOutSize);
}

}  // namespace

bool NfcWriteApp::ensureReady() {
  if (Nfc.isReady()) {
    return true;
  }
  return Nfc.begin();
}

NfcWriteResult NfcWriteApp::runWrite(bool (*writer)(const char*, char*, size_t), const char* payload,
                                     const char* type, const char* summary) {
  NfcWriteResult result;
  strlcpy(result.writeType, type, sizeof(result.writeType));
  strlcpy(result.summary, summary, sizeof(result.summary));

  if (!payload || payload[0] == '\0') {
    strncpy(result.error, "Nothing to write", sizeof(result.error) - 1);
    return result;
  }

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!writer(payload, result.error, sizeof(result.error))) {
    if (result.error[0] == '\0') {
      strncpy(result.error, "Write failed — keep tag on PN532", sizeof(result.error) - 1);
    }
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::writeOk();
  Log.activity("nfc", type, summary);
  NfcHist.append(type, "tag", summary);
  return result;
}

NfcWriteResult NfcWriteApp::writeUri(const char* url) {
  return runWrite(writeUriAdapter, url, "write", url);
}

NfcWriteResult NfcWriteApp::writeText(const char* text) {
  return runWrite(writeTextAdapter, text, "write_text", text);
}

String NfcWriteApp::buildNfcVcard(const HackCardConfig& profile) const {
  String compact;
  compact += "BEGIN:VCARD\r\n";
  compact += "VERSION:3.0\r\n";
  compact += "FN:";
  compact += profile.ownerName;
  compact += "\r\n";
  if (profile.title[0] != '\0') {
    compact += "TITLE:";
    compact += profile.title;
    compact += "\r\n";
  }
  if (profile.company[0] != '\0') {
    compact += "ORG:";
    compact += profile.company;
    compact += "\r\n";
  }
  if (profile.phone[0] != '\0') {
    compact += "TEL:";
    compact += profile.phone;
    compact += "\r\n";
  }
  if (profile.email[0] != '\0') {
    compact += "EMAIL:";
    compact += profile.email;
    compact += "\r\n";
  }
  if (profile.website[0] != '\0') {
    compact += "URL:";
    compact += profile.website;
    compact += "\r\n";
  }
  compact += "END:VCARD\r\n";

  if (compact.length() <= 130) {
    return compact;
  }

  return buildVcard(profile);
}

NfcWriteResult NfcWriteApp::writeVcard(const HackCardConfig& profile) {
  NfcWriteResult result;
  const String vcard = buildNfcVcard(profile);
  if (vcard.isEmpty() || profile.ownerName[0] == '\0') {
    strncpy(result.error, "Profile empty — set name in /profile", sizeof(result.error) - 1);
    return result;
  }

  static char vcardBuf[512];
  strlcpy(vcardBuf, vcard.c_str(), sizeof(vcardBuf));

  return runWrite(writeVcardAdapter, vcardBuf, "write_vcard", profile.ownerName);
}

NfcEraseResult NfcWriteApp::eraseTag(const char* labPin) {
  NfcEraseResult result;

  if (!nfcLabPinOk(labPin)) {
    strncpy(result.error, "Lab PIN required — nfc erase <pin>", sizeof(result.error) - 1);
    return result;
  }

  if (!ensureReady()) {
    strncpy(result.error, "PN532 not ready", sizeof(result.error) - 1);
    NfcFeedback::error();
    return result;
  }

  NfcFeedback::scanStart();
  if (!Nfc.eraseType2UserData(result.error, sizeof(result.error))) {
    if (result.error[0] == '\0') {
      strncpy(result.error, "Erase failed — tag must be Type 2 (NTAG/Ultralight)", sizeof(result.error) - 1);
    }
    NfcFeedback::error();
    return result;
  }

  result.success = true;
  NfcFeedback::writeOk();
  Log.activity("nfc", "erase", "ok");
  NfcHist.append("erase", "tag", "user-pages-cleared");
  return result;
}

String NfcWriteApp::writeJson(const NfcWriteResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"type\":\"";
  json += result.writeType;
  json += "\",\"summary\":\"";
  json += result.summary;
  json += "\",\"url\":\"";
  json += result.summary;
  json += "\",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}

String NfcWriteApp::eraseJson(const NfcEraseResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  if (result.success) {
    json += ",\"message\":\"Type 2 user pages cleared\"";
  }
  json += ",\"error\":\"";
  json += result.error;
  json += "\"}";
  return json;
}
