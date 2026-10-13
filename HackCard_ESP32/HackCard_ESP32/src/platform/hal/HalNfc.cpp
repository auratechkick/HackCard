#include "HalNfc.h"

#include "../../core/HcSerial.h"

#include "config/app_registry.h"
#include "config/board_config.h"

#include <cstring>

HalNfc Nfc;

#if APP_NFC_READ

#include <Wire.h>
#include <Adafruit_PN532.h>

namespace {

Adafruit_PN532 sPn532(PIN_NFC_IRQ, 0xFF, &Wire);

const char* uidToHex(char* dest, size_t destSize, const uint8_t* uid, uint8_t uidLength) {
  dest[0] = '\0';
  if (!dest || destSize < 3 || !uid) {
    return dest;
  }

  size_t pos = 0;
  for (uint8_t i = 0; i < uidLength; i++) {
    if (pos + 3 >= destSize) {
      break;
    }
    if (i > 0) {
      dest[pos++] = ':';
    }
    pos += snprintf(dest + pos, destSize - pos, "%02X", uid[i]);
  }
  dest[destSize - 1] = '\0';
  return dest;
}

}  // namespace

bool HalNfc::begin() {
  Wire.begin(PIN_NFC_SDA, PIN_NFC_SCL);
  delay(50);

  // Adafruit_PN532 v2.x: I2C address fixed at 0x24 in library (matches board_config)
  if (!sPn532.begin()) {
    _ready = false;
    strncpy(_chipInfo, "PN532 not found on I2C", sizeof(_chipInfo) - 1);
    HcSerial.println("[nfc] ERROR: PN532 not detected");
    return false;
  }

  const uint32_t version = sPn532.getFirmwareVersion();
  if (!version) {
    _ready = false;
    strncpy(_chipInfo, "PN532 firmware read failed", sizeof(_chipInfo) - 1);
    HcSerial.println("[nfc] ERROR: Could not read PN532 firmware");
    return false;
  }

  _ready = true;
  snprintf(_chipInfo, sizeof(_chipInfo), "PN532 IC %lu.%lu", (version >> 24) & 0xFF, (version >> 16) & 0xFF);
  sPn532.SAMConfig();
  HcSerial.printf("[nfc] Ready — %s (SDA=%d SCL=%d)\n", _chipInfo, PIN_NFC_SDA, PIN_NFC_SCL);
  return true;
}

bool HalNfc::readTag(NfcTagInfo& out, uint32_t timeoutMs) {
  out = NfcTagInfo();
  out.chipReady = _ready;

  if (!_ready) {
    strncpy(out.error, "NFC chip not ready", sizeof(out.error) - 1);
    return false;
  }

  uint8_t uid[10] = {0};
  uint8_t uidLength = 0;

  const bool found = sPn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeoutMs);
  if (!found || uidLength == 0) {
    strncpy(out.error, "No tag detected — hold card near PN532", sizeof(out.error) - 1);
    return false;
  }

  out.detected = true;
  out.uidLength = uidLength;
  memcpy(out.uidRaw, uid, uidLength);
  uidToHex(out.uidHex, sizeof(out.uidHex), uid, uidLength);
  inferTagType(out);
  return true;
}

bool HalNfc::readType2PageInternal(uint8_t page, uint8_t buffer[4]) {
  if (sPn532.ntag2xx_ReadPage(page, buffer) == 1) {
    return true;
  }
  return sPn532.mifareultralight_ReadPage(page, buffer) == 1;
}

bool HalNfc::writeType2PageInternal(uint8_t page, const uint8_t buffer[4]) {
  if (sPn532.ntag2xx_WritePage(page, const_cast<uint8_t*>(buffer)) == 1) {
    return true;
  }
  return sPn532.mifareultralight_WritePage(page, const_cast<uint8_t*>(buffer)) == 1;
}

bool HalNfc::readType2DataAreaBytes(uint16_t& dataAreaBytes, char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] %s\n", msg);
    return false;
  };

  uint8_t cc[4] = {0};
  if (!readType2PageInternal(3, cc)) {
    return setError("Could not read capability container (page 3)");
  }

  if (cc[0] != 0xE1 || cc[1] != 0x10) {
    return setError("Tag is not NDEF formatted (page 3 must start E1 10)");
  }

  dataAreaBytes = static_cast<uint16_t>(cc[2]) * 8U;
  if (dataAreaBytes < 48) {
    return setError("Tag data area too small for NDEF URI");
  }

  return true;
}

bool HalNfc::eraseType2UserPages(uint16_t dataAreaBytes, char* errorOut, size_t errorOutSize) {
  const uint8_t startPage = 4;
  const uint8_t endPage = static_cast<uint8_t>(startPage + (dataAreaBytes / 4U) - 1U);
  uint8_t blank[4] = {0, 0, 0, 0};

  for (uint8_t page = startPage; page <= endPage; page++) {
    if (!writeType2PageInternal(page, blank)) {
      if (errorOut && errorOutSize > 0) {
        snprintf(errorOut, errorOutSize, "Failed to erase page %u", page);
        errorOut[errorOutSize - 1] = '\0';
      }
      HcSerial.printf("[nfc] Erase failed on page %u\n", page);
      return false;
    }
  }

  return true;
}

bool HalNfc::readType2Page(uint8_t page, uint8_t buffer[4]) {
  if (!_ready || !buffer) {
    return false;
  }
  return readType2PageInternal(page, buffer);
}

bool HalNfc::dumpType2Tag(NfcDumpData& out, uint8_t maxPages, uint32_t timeoutMs) {
  out = NfcDumpData();
  out.tag.chipReady = _ready;

  if (!_ready) {
    strncpy(out.error, "NFC chip not ready", sizeof(out.error) - 1);
    return false;
  }

  if (!readTag(out.tag, timeoutMs)) {
    strncpy(out.error, out.tag.error, sizeof(out.error) - 1);
    return false;
  }

  if (maxPages > 48) {
    maxPages = 48;
  }

  uint8_t failStreak = 0;
  for (uint8_t page = 0; page < maxPages; page++) {
    if (!readType2PageInternal(page, out.pages[out.pageCount])) {
      failStreak++;
      if (failStreak >= 3 && out.pageCount > 0) {
        break;
      }
      continue;
    }
    failStreak = 0;
    out.pageCount++;
  }

  if (out.pageCount == 0) {
    strncpy(out.error, "Type 2 page read failed — tag may be Mifare Classic", sizeof(out.error) - 1);
    return false;
  }

  out.success = true;
  return true;
}

bool HalNfc::writeType2NdefMessage(const uint8_t* ndefMsg, size_t ndefLen, uint16_t dataAreaBytes,
                                     char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] NDEF write: %s\n", msg);
    return false;
  };

  if (!ndefMsg || ndefLen == 0) {
    return setError("Empty NDEF message");
  }

  size_t tlvLen = 2;
  if (ndefLen <= 254) {
    tlvLen += ndefLen;
  } else {
    tlvLen += 3 + ndefLen;
  }
  const size_t totalBytes = 5 + tlvLen + 1;
  if (totalBytes > dataAreaBytes) {
    char msg[72];
    snprintf(msg, sizeof(msg), "NDEF too large (%u bytes) for tag area (%u)", totalBytes, dataAreaBytes);
    return setError(msg);
  }

  uint8_t stream[512] = {0};
  size_t pos = 0;
  stream[pos++] = 0x01;
  stream[pos++] = 0x03;
  stream[pos++] = 0xA0;
  stream[pos++] = 0x10;
  stream[pos++] = 0x44;
  stream[pos++] = 0x03;
  if (ndefLen <= 254) {
    stream[pos++] = static_cast<uint8_t>(ndefLen);
  } else {
    stream[pos++] = 0xFF;
    stream[pos++] = static_cast<uint8_t>((ndefLen >> 8) & 0xFF);
    stream[pos++] = static_cast<uint8_t>(ndefLen & 0xFF);
  }
  memcpy(stream + pos, ndefMsg, ndefLen);
  pos += ndefLen;
  stream[pos++] = 0xFE;

  const uint8_t endPage = static_cast<uint8_t>(4 + (dataAreaBytes / 4U) - 1U);
  for (uint8_t page = 4; page <= endPage; page++) {
    const size_t offset = (page - 4U) * 4U;
    uint8_t pageData[4] = {0, 0, 0, 0};
    if (offset < pos) {
      const size_t chunk = (pos - offset >= 4) ? 4 : (pos - offset);
      memcpy(pageData, stream + offset, chunk);
    }
    if (!writeType2PageInternal(page, pageData)) {
      char msg[48];
      snprintf(msg, sizeof(msg), "Failed writing page %u", page);
      return setError(msg);
    }
  }

  return true;
}

namespace {

size_t buildNdefTextRecord(const char* text, uint8_t* out, size_t outMax) {
  if (!text || !out) {
    return 0;
  }

  const char* lang = "en";
  const uint8_t langLen = 2;
  const size_t textLen = strlen(text);
  const size_t payloadLen = 1U + langLen + textLen;
  const size_t recordLen = 4U + payloadLen;

  if (recordLen > outMax || textLen == 0) {
    return 0;
  }

  out[0] = 0xD1;
  out[1] = 0x01;
  out[2] = static_cast<uint8_t>(payloadLen);
  out[3] = 0x54;
  out[4] = langLen;
  memcpy(out + 5, lang, langLen);
  memcpy(out + 5 + langLen, text, textLen);
  return recordLen;
}

size_t buildNdefMimeRecord(const char* mimeType, const uint8_t* payload, size_t payloadLen, uint8_t* out,
                           size_t outMax) {
  if (!mimeType || !payload || !out || payloadLen == 0) {
    return 0;
  }

  const size_t mimeLen = strlen(mimeType);
  if (mimeLen == 0 || mimeLen > 255) {
    return 0;
  }

  if (payloadLen <= 255) {
    const size_t recordLen = 3U + mimeLen + payloadLen;
    if (recordLen > outMax) {
      return 0;
    }
    out[0] = 0xD2;
    out[1] = static_cast<uint8_t>(mimeLen);
    out[2] = static_cast<uint8_t>(payloadLen);
    memcpy(out + 3, mimeType, mimeLen);
    memcpy(out + 3 + mimeLen, payload, payloadLen);
    return recordLen;
  }

  const size_t recordLen = 7U + mimeLen + payloadLen;
  if (recordLen > outMax) {
    return 0;
  }
  out[0] = 0xC2;
  out[1] = static_cast<uint8_t>(mimeLen);
  out[2] = static_cast<uint8_t>((payloadLen >> 24) & 0xFF);
  out[3] = static_cast<uint8_t>((payloadLen >> 16) & 0xFF);
  out[4] = static_cast<uint8_t>((payloadLen >> 8) & 0xFF);
  out[5] = static_cast<uint8_t>(payloadLen & 0xFF);
  memcpy(out + 6, mimeType, mimeLen);
  memcpy(out + 6 + mimeLen, payload, payloadLen);
  return recordLen;
}

}  // namespace

bool HalNfc::writeNdefText(const char* text, char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Write text: %s\n", msg);
    return false;
  };

  if (!_ready || !text || text[0] == '\0') {
    return setError("Text missing");
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    return setError(tag.error[0] ? tag.error : "No tag detected");
  }

  uint16_t dataAreaBytes = 0;
  if (!readType2DataAreaBytes(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  uint8_t ndefMsg[240] = {0};
  const size_t ndefLen = buildNdefTextRecord(text, ndefMsg, sizeof(ndefMsg));
  if (ndefLen == 0) {
    return setError("Text too long for tag");
  }

  if (!eraseType2UserPages(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  if (!writeType2NdefMessage(ndefMsg, ndefLen, dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  HcSerial.printf("[nfc] Write text OK — %u chars\n", strlen(text));
  return true;
}

bool HalNfc::writeNdefVcard(const char* vcard, char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Write vCard: %s\n", msg);
    return false;
  };

  if (!_ready || !vcard || vcard[0] == '\0') {
    return setError("vCard missing");
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    return setError(tag.error[0] ? tag.error : "No tag detected");
  }

  uint16_t dataAreaBytes = 0;
  if (!readType2DataAreaBytes(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  const size_t vcardLen = strlen(vcard);
  uint8_t ndefMsg[512] = {0};
  const size_t ndefLen =
      buildNdefMimeRecord("text/vcard", reinterpret_cast<const uint8_t*>(vcard), vcardLen, ndefMsg, sizeof(ndefMsg));
  if (ndefLen == 0) {
    return setError("vCard too long — use NTAG215/216 or shorten profile");
  }

  if (!eraseType2UserPages(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  if (!writeType2NdefMessage(ndefMsg, ndefLen, dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  HcSerial.println("[nfc] Write vCard OK");
  return true;
}

bool HalNfc::writeNdefUri(const char* url, char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Write: %s\n", msg);
    return false;
  };

  if (!_ready || !url || url[0] == '\0') {
    return setError("NFC not ready or URL missing");
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    return setError(tag.error[0] ? tag.error : "No tag detected");
  }

  uint16_t dataAreaBytes = 0;
  if (!readType2DataAreaBytes(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  char urlBuf[128];
  strlcpy(urlBuf, url, sizeof(urlBuf));

  uint8_t prefix = NDEF_URIPREFIX_NONE;
  char* writePtr = urlBuf;
  if (strncmp(urlBuf, "https://", 8) == 0) {
    prefix = NDEF_URIPREFIX_HTTPS;
    writePtr = urlBuf + 8;
  } else if (strncmp(urlBuf, "http://", 7) == 0) {
    prefix = NDEF_URIPREFIX_HTTP;
    writePtr = urlBuf + 7;
  }

  const size_t urlLen = strlen(writePtr);
  constexpr uint8_t kNdefOverhead = 12;
  if (urlLen < 1) {
    return setError("URL too short after prefix strip");
  }
  if (urlLen + 1 > (dataAreaBytes - kNdefOverhead)) {
    char msg[64];
    snprintf(msg, sizeof(msg), "URL too long for tag (%u byte area)", dataAreaBytes);
    return setError(msg);
  }

  if (!eraseType2UserPages(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  const uint8_t dataLenArg = (dataAreaBytes > 255) ? 255 : static_cast<uint8_t>(dataAreaBytes);
  if (sPn532.ntag2xx_WriteNDEFURI(prefix, writePtr, dataLenArg) != 1) {
    return setError("PN532 NDEF write failed — keep tag still on antenna");
  }

  HcSerial.printf("[nfc] Write OK — %u byte area, prefix 0x%02X\n", dataAreaBytes, prefix);
  return true;
}

bool HalNfc::eraseType2UserData(char* errorOut, size_t errorOutSize) {
  if (!_ready) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, "NFC chip not ready", errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    return false;
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, tag.error[0] ? tag.error : "No tag detected", errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    return false;
  }

  uint16_t dataAreaBytes = 0;
  if (!readType2DataAreaBytes(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  if (!eraseType2UserPages(dataAreaBytes, errorOut, errorOutSize)) {
    return false;
  }

  HcSerial.printf("[nfc] Erase OK — cleared %u user bytes\n", dataAreaBytes);
  return true;
}

bool HalNfc::restoreType2Pages(const NfcDumpData& dump, uint8_t startPage, char* errorOut,
                               size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Restore: %s\n", msg);
    return false;
  };

  if (!_ready || !dump.success || dump.pageCount <= startPage) {
    return setError("Invalid dump or NFC not ready");
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    return setError(tag.error[0] ? tag.error : "No destination tag detected");
  }

  for (uint8_t page = startPage; page < dump.pageCount; page++) {
    if (!writeType2PageInternal(page, dump.pages[page])) {
      char msg[48];
      snprintf(msg, sizeof(msg), "Failed writing page %u", page);
      return setError(msg);
    }
  }

  HcSerial.printf("[nfc] Restore OK — %u pages from page %u\n", dump.pageCount - startPage, startPage);
  return true;
}

bool HalNfc::killType2Tag(char* errorOut, size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Kill: %s\n", msg);
    return false;
  };

  if (!_ready) {
    return setError("NFC chip not ready");
  }

  NfcTagInfo tag;
  if (!readTag(tag, 1500)) {
    return setError(tag.error[0] ? tag.error : "No tag detected");
  }

  uint8_t page2[4] = {0};
  if (!readType2PageInternal(2, page2)) {
    return setError("Could not read lock page (page 2)");
  }

  page2[2] = 0xFF;
  page2[3] = 0xFF;
  if (!writeType2PageInternal(2, page2)) {
    return setError("Failed to set static lock bytes");
  }

  uint8_t page40[4] = {0};
  if (readType2PageInternal(40, page40)) {
    page40[0] = 0xFF;
    page40[1] = 0xFF;
    page40[2] = 0xFF;
    page40[3] = 0xFF;
    writeType2PageInternal(40, page40);
  }

  uint16_t dataAreaBytes = 0;
  if (readType2DataAreaBytes(dataAreaBytes, errorOut, errorOutSize)) {
    eraseType2UserPages(dataAreaBytes, errorOut, errorOutSize);
  }

  HcSerial.println("[nfc] Kill OK — tag locked and user data cleared");
  return true;
}

bool HalNfc::parseClassicKeyHex(const char* keyHex, uint8_t keyOut[6], char* errorOut, size_t errorOutSize) {
  if (!keyHex || !keyOut) {
    return false;
  }

  String cleaned = keyHex;
  cleaned.trim();
  cleaned.replace(":", "");
  cleaned.replace(" ", "");
  cleaned.toUpperCase();

  if (cleaned.length() != 12) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, "Key must be 12 hex chars (6 bytes)", errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    return false;
  }

  for (uint8_t i = 0; i < 6; i++) {
    const String byteStr = cleaned.substring(i * 2, i * 2 + 2);
    keyOut[i] = static_cast<uint8_t>(strtoul(byteStr.c_str(), nullptr, 16));
  }

  return true;
}

bool HalNfc::authenticateClassicBlock(const NfcTagInfo& tag, uint8_t blockNumber, const uint8_t key[6],
                                      uint8_t keyType) {
  if (tag.uidLength != 4) {
    return false;
  }

  return sPn532.mifareclassic_AuthenticateBlock(const_cast<uint8_t*>(tag.uidRaw), tag.uidLength, blockNumber,
                                                keyType, const_cast<uint8_t*>(key)) == 1;
}

bool HalNfc::readClassicBlock(uint8_t blockNumber, uint8_t data[16]) {
  return sPn532.mifareclassic_ReadDataBlock(blockNumber, data) == 1;
}

bool HalNfc::writeClassicBlock(uint8_t blockNumber, const uint8_t data[16]) {
  return sPn532.mifareclassic_WriteDataBlock(blockNumber, const_cast<uint8_t*>(data)) == 1;
}

bool HalNfc::dumpClassicTag(const uint8_t key[6], NfcClassicDump& out, uint8_t maxSectors, uint32_t timeoutMs) {
  out = NfcClassicDump();
  out.sectorCount = maxSectors > 16 ? 16 : maxSectors;

  if (!_ready || !key) {
    strncpy(out.error, "NFC not ready", sizeof(out.error) - 1);
    return false;
  }

  if (!readTag(out.tag, timeoutMs)) {
    strncpy(out.error, out.tag.error, sizeof(out.error) - 1);
    return false;
  }

  if (out.tag.uidLength != 4) {
    strncpy(out.error, "Not a Mifare Classic 4-byte UID tag", sizeof(out.error) - 1);
    return false;
  }

  snprintf(out.keyHex, sizeof(out.keyHex), "%02X%02X%02X%02X%02X%02X", key[0], key[1], key[2], key[3], key[4],
           key[5]);
  uint8_t readableSectors = 0;

  for (uint8_t sector = 0; sector < out.sectorCount; sector++) {
    const uint8_t firstBlock = sector * 4;
    bool authed = authenticateClassicBlock(out.tag, firstBlock, key, 0);
    if (!authed) {
      authed = authenticateClassicBlock(out.tag, firstBlock, key, 1);
    }
    if (!authed) {
      continue;
    }

    bool sectorOk = true;
    for (uint8_t blockIdx = 0; blockIdx < 3; blockIdx++) {
      if (!readClassicBlock(firstBlock + blockIdx, out.sectors[sector].blocks[blockIdx])) {
        sectorOk = false;
        break;
      }
    }

    if (sectorOk) {
      out.sectors[sector].readable = true;
      readableSectors++;
    }
  }

  if (readableSectors == 0) {
    strncpy(out.error, "No sectors readable — wrong key?", sizeof(out.error) - 1);
    return false;
  }

  out.success = true;
  return true;
}

bool HalNfc::cloneClassicDump(const NfcClassicDump& dump, const uint8_t key[6], char* errorOut,
                              size_t errorOutSize) {
  auto setError = [&](const char* msg) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, msg, errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    HcSerial.printf("[nfc] Classic clone: %s\n", msg);
    return false;
  };

  if (!_ready || !dump.success || !key) {
    return setError("Invalid dump or NFC not ready");
  }

  NfcTagInfo dest;
  if (!readTag(dest, 1500)) {
    return setError(dest.error[0] ? dest.error : "No destination tag detected");
  }

  if (dest.uidLength != 4) {
    return setError("Destination must be Mifare Classic (4-byte UID)");
  }

  uint8_t writtenBlocks = 0;
  for (uint8_t sector = 0; sector < dump.sectorCount; sector++) {
    if (!dump.sectors[sector].readable) {
      continue;
    }

    const uint8_t firstBlock = sector * 4;
    bool authed = authenticateClassicBlock(dest, firstBlock, key, 0);
    if (!authed) {
      authed = authenticateClassicBlock(dest, firstBlock, key, 1);
    }
    if (!authed) {
      return setError("Auth failed on destination tag");
    }

    for (uint8_t blockIdx = 0; blockIdx < 3; blockIdx++) {
      if (sector == 0 && blockIdx == 0) {
        continue;
      }

      if (!writeClassicBlock(firstBlock + blockIdx, dump.sectors[sector].blocks[blockIdx])) {
        char msg[48];
        snprintf(msg, sizeof(msg), "Write failed block %u", firstBlock + blockIdx);
        return setError(msg);
      }
      writtenBlocks++;
    }
  }

  if (writtenBlocks == 0) {
    return setError("No writable blocks in dump");
  }

  HcSerial.printf("[nfc] Classic clone OK — %u blocks written\n", writtenBlocks);
  return true;
}

bool HalNfc::scanClassicKey(uint8_t sector, char* keyHexOut, size_t keyHexOutSize) {
  if (!keyHexOut || keyHexOutSize < 13) {
    return false;
  }

  NfcTagInfo tag;
  if (!_ready || !readTag(tag, 1500) || tag.uidLength != 4) {
    return false;
  }

  static const uint8_t kCommonKeys[][6] = {
      {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
      {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5},
      {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5},
      {0x4D, 0x3A, 0x99, 0xC3, 0x51, 0xDD},
      {0x1A, 0x98, 0x2C, 0x7E, 0x45, 0x9A},
      {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
      {0x71, 0x4C, 0x5C, 0x88, 0x6E, 0x97},
      {0x58, 0x7E, 0xE5, 0xF9, 0x35, 0x0F},
  };

  const uint8_t firstBlock = sector * 4;
  for (size_t i = 0; i < sizeof(kCommonKeys) / sizeof(kCommonKeys[0]); i++) {
    if (authenticateClassicBlock(tag, firstBlock, kCommonKeys[i], 0) ||
        authenticateClassicBlock(tag, firstBlock, kCommonKeys[i], 1)) {
      snprintf(keyHexOut, keyHexOutSize, "%02X%02X%02X%02X%02X%02X", kCommonKeys[i][0], kCommonKeys[i][1],
               kCommonKeys[i][2], kCommonKeys[i][3], kCommonKeys[i][4], kCommonKeys[i][5]);
      return true;
    }
  }

  return false;
}

namespace {

void bytesToHex(char* dest, size_t destSize, const uint8_t* data, size_t len) {
  dest[0] = '\0';
  if (!dest || !data || destSize < 3) {
    return;
  }
  size_t pos = 0;
  for (size_t i = 0; i < len; i++) {
    if (pos + 3 >= destSize) {
      break;
    }
    if (i > 0) {
      dest[pos++] = ' ';
    }
    pos += snprintf(dest + pos, destSize - pos, "%02X", data[i]);
  }
}

void tryExtractText(const uint8_t* data, size_t len, char* out, size_t outSize) {
  out[0] = '\0';
  for (size_t i = 0; i + 4 < len; i++) {
    if (memcmp(data + i, "http", 4) == 0 || memcmp(data + i, "www.", 4) == 0 ||
        memcmp(data + i, "BEGIN:VCARD", 11) == 0) {
      size_t pos = 0;
      for (size_t j = i; j < len && pos + 1 < outSize; j++) {
        const char c = static_cast<char>(data[j]);
        if (c >= 32 && c <= 126) {
          out[pos++] = c;
        } else if (pos > 0) {
          break;
        }
      }
      out[outSize - 1] = '\0';
      return;
    }
  }
}

}  // namespace

bool HalNfc::probeUltralightPages(NfcTagDetail& out) {
  uint8_t page0[4] = {0};
  if (!readType2PageInternal(0, page0)) {
    return false;
  }

  strncpy(out.family, "NTAG / Ultralight (Type 2)", sizeof(out.family) - 1);
  snprintf(out.memoryHint, sizeof(out.memoryHint), "Page0: %02X %02X %02X %02X", page0[0], page0[1], page0[2],
           page0[3]);

  uint8_t buffer[16] = {0};
  for (uint8_t page = 4; page <= 6; page++) {
    if (!readType2PageInternal(page, buffer + ((page - 4) * 4))) {
      break;
    }
  }

  tryExtractText(buffer, sizeof(buffer), out.ndefPreview, sizeof(out.ndefPreview));
  if (out.ndefPreview[0] != '\0') {
    strncat(out.memoryHint, " | NDEF found", sizeof(out.memoryHint) - strlen(out.memoryHint) - 1);
  }
  return true;
}

void HalNfc::inferFamily(NfcTagDetail& out) const {
  if (out.family[0] != '\0') {
    return;
  }

  switch (out.uidLength) {
    case 4:
      strncpy(out.family, "Mifare Classic (Type 2K/4K likely)", sizeof(out.family) - 1);
      strncpy(out.memoryHint, "Use nfc classic dump <key> or nfc keys scan", sizeof(out.memoryHint) - 1);
      break;
    case 7:
      strncpy(out.family, "NTAG / Ultralight (7-byte UID)", sizeof(out.family) - 1);
      break;
    case 10:
      strncpy(out.family, "ISO14443-4 tag", sizeof(out.family) - 1);
      break;
    default:
      strncpy(out.family, "Unknown ISO14443A tag", sizeof(out.family) - 1);
      break;
  }
}

bool HalNfc::readTagDetail(NfcTagDetail& out, uint32_t timeoutMs) {
  if (!readTag(out, timeoutMs)) {
    return false;
  }

  if (!probeUltralightPages(out)) {
    inferFamily(out);
  }

  if (out.tagType[0] == '\0') {
    inferTagType(out);
  }

  return true;
}

void HalNfc::inferTagType(NfcTagInfo& out) const {
  switch (out.uidLength) {
    case 4:
      strncpy(out.tagType, "Mifare Classic (4-byte UID)", sizeof(out.tagType) - 1);
      break;
    case 7:
      strncpy(out.tagType, "NTAG / Ultralight (7-byte UID)", sizeof(out.tagType) - 1);
      break;
    case 10:
      strncpy(out.tagType, "ISO14443-4 (10-byte UID)", sizeof(out.tagType) - 1);
      break;
    default:
      snprintf(out.tagType, sizeof(out.tagType), "Unknown (%u-byte UID)", out.uidLength);
      break;
  }
}

#else

bool HalNfc::begin() {
  strncpy(_chipInfo, "NFC app disabled", sizeof(_chipInfo) - 1);
  return false;
}

bool HalNfc::readTag(NfcTagInfo& out, uint32_t timeoutMs) {
  (void)timeoutMs;
  out = NfcTagInfo();
  strncpy(out.error, "APP_NFC_READ disabled", sizeof(out.error) - 1);
  return false;
}

bool HalNfc::readTagDetail(NfcTagDetail& out, uint32_t timeoutMs) {
  (void)timeoutMs;
  out = NfcTagDetail();
  strncpy(out.error, "APP_NFC_READ disabled", sizeof(out.error) - 1);
  return false;
}

bool HalNfc::readType2Page(uint8_t page, uint8_t buffer[4]) {
  (void)page;
  (void)buffer;
  return false;
}

bool HalNfc::dumpType2Tag(NfcDumpData& out, uint8_t maxPages, uint32_t timeoutMs) {
  (void)maxPages;
  (void)timeoutMs;
  out = NfcDumpData();
  strncpy(out.error, "APP_NFC_READ disabled", sizeof(out.error) - 1);
  return false;
}

bool HalNfc::writeNdefUri(const char* url, char* errorOut, size_t errorOutSize) {
  (void)url;
  if (errorOut && errorOutSize > 0) {
    strncpy(errorOut, "APP_NFC_READ disabled", errorOutSize - 1);
    errorOut[errorOutSize - 1] = '\0';
  }
  return false;
}

bool HalNfc::writeNdefText(const char* text, char* errorOut, size_t errorOutSize) {
  (void)text;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::writeNdefVcard(const char* vcard, char* errorOut, size_t errorOutSize) {
  (void)vcard;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::writeType2NdefMessage(const uint8_t* ndefMsg, size_t ndefLen, uint16_t dataAreaBytes,
                                   char* errorOut, size_t errorOutSize) {
  (void)ndefMsg;
  (void)ndefLen;
  (void)dataAreaBytes;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::eraseType2UserData(char* errorOut, size_t errorOutSize) {
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::restoreType2Pages(const NfcDumpData& dump, uint8_t startPage, char* errorOut, size_t errorOutSize) {
  (void)dump;
  (void)startPage;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::killType2Tag(char* errorOut, size_t errorOutSize) {
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::parseClassicKeyHex(const char* keyHex, uint8_t keyOut[6], char* errorOut, size_t errorOutSize) {
  (void)keyHex;
  (void)keyOut;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::dumpClassicTag(const uint8_t key[6], NfcClassicDump& out, uint8_t maxSectors, uint32_t timeoutMs) {
  (void)key;
  (void)maxSectors;
  (void)timeoutMs;
  out = NfcClassicDump();
  strncpy(out.error, "APP_NFC_READ disabled", sizeof(out.error) - 1);
  return false;
}

bool HalNfc::cloneClassicDump(const NfcClassicDump& dump, const uint8_t key[6], char* errorOut, size_t errorOutSize) {
  (void)dump;
  (void)key;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::scanClassicKey(uint8_t sector, char* keyHexOut, size_t keyHexOutSize) {
  (void)sector;
  (void)keyHexOut;
  (void)keyHexOutSize;
  return false;
}

bool HalNfc::writeType2PageInternal(uint8_t page, const uint8_t buffer[4]) {
  (void)page;
  (void)buffer;
  return false;
}

bool HalNfc::readType2DataAreaBytes(uint16_t& dataAreaBytes, char* errorOut, size_t errorOutSize) {
  (void)dataAreaBytes;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

bool HalNfc::eraseType2UserPages(uint16_t dataAreaBytes, char* errorOut, size_t errorOutSize) {
  (void)dataAreaBytes;
  (void)errorOut;
  (void)errorOutSize;
  return false;
}

void HalNfc::inferTagType(NfcTagInfo& out) const {
  (void)out;
}

void HalNfc::inferFamily(NfcTagDetail& out) const {
  (void)out;
}

bool HalNfc::probeUltralightPages(NfcTagDetail& out) {
  (void)out;
  return false;
}

#endif
