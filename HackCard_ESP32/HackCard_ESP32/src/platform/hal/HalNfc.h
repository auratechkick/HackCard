#pragma once

#include <Arduino.h>

struct NfcTagInfo {
  bool detected = false;
  bool chipReady = false;
  uint8_t uidLength = 0;
  uint8_t uidRaw[10] = {0};
  char uidHex[64] = {0};
  char tagType[48] = {0};
  char error[64] = {0};
};

struct NfcTagDetail : public NfcTagInfo {
  char family[48] = {0};
  char ndefPreview[128] = {0};
  char memoryHint[64] = {0};
};

struct NfcDumpData {
  bool success = false;
  NfcTagInfo tag;
  uint8_t pageCount = 0;
  uint8_t pages[48][4] = {0};
  char error[64] = {0};
};

struct NfcClassicSector {
  bool readable = false;
  uint8_t blocks[3][16] = {0};
};

struct NfcClassicDump {
  bool success = false;
  NfcTagInfo tag;
  uint8_t sectorCount = 16;
  NfcClassicSector sectors[16];
  char keyHex[13] = {0};
  char error[64] = {0};
};

class HalNfc {
public:
  bool begin();
  bool isReady() const { return _ready; }
  const char* getChipInfo() const { return _chipInfo; }

  bool readTag(NfcTagInfo& out, uint32_t timeoutMs = 1000);
  bool readTagDetail(NfcTagDetail& out, uint32_t timeoutMs = 1500);
  bool readType2Page(uint8_t page, uint8_t buffer[4]);
  bool dumpType2Tag(NfcDumpData& out, uint8_t maxPages = 45, uint32_t timeoutMs = 1500);
  bool writeNdefUri(const char* url, char* errorOut = nullptr, size_t errorOutSize = 0);
  bool writeNdefText(const char* text, char* errorOut = nullptr, size_t errorOutSize = 0);
  bool writeNdefVcard(const char* vcard, char* errorOut = nullptr, size_t errorOutSize = 0);
  bool eraseType2UserData(char* errorOut = nullptr, size_t errorOutSize = 0);
  bool restoreType2Pages(const NfcDumpData& dump, uint8_t startPage, char* errorOut = nullptr,
                         size_t errorOutSize = 0);
  bool killType2Tag(char* errorOut = nullptr, size_t errorOutSize = 0);

  bool parseClassicKeyHex(const char* keyHex, uint8_t keyOut[6], char* errorOut = nullptr,
                          size_t errorOutSize = 0);
  bool dumpClassicTag(const uint8_t key[6], NfcClassicDump& out, uint8_t maxSectors = 16,
                      uint32_t timeoutMs = 1500);
  bool cloneClassicDump(const NfcClassicDump& dump, const uint8_t key[6], char* errorOut = nullptr,
                        size_t errorOutSize = 0);
  bool scanClassicKey(uint8_t sector, char* keyHexOut, size_t keyHexOutSize);

private:
  void inferTagType(NfcTagInfo& out) const;
  void inferFamily(NfcTagDetail& out) const;
  bool probeUltralightPages(NfcTagDetail& out);
  bool readType2PageInternal(uint8_t page, uint8_t buffer[4]);
  bool writeType2PageInternal(uint8_t page, const uint8_t buffer[4]);
  bool readType2DataAreaBytes(uint16_t& dataAreaBytes, char* errorOut, size_t errorOutSize);
  bool eraseType2UserPages(uint16_t dataAreaBytes, char* errorOut, size_t errorOutSize);
  bool writeType2NdefMessage(const uint8_t* ndefMsg, size_t ndefLen, uint16_t dataAreaBytes, char* errorOut,
                             size_t errorOutSize);
  bool authenticateClassicBlock(const NfcTagInfo& tag, uint8_t blockNumber, const uint8_t key[6],
                                uint8_t keyType);
  bool readClassicBlock(uint8_t blockNumber, uint8_t data[16]);
  bool writeClassicBlock(uint8_t blockNumber, const uint8_t data[16]);

  bool _ready = false;
  char _chipInfo[48] = "not initialized";
};

extern HalNfc Nfc;

bool nfcLabPinOk(const char* pin);
