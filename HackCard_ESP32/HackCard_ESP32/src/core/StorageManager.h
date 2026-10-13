#pragma once

#include <Arduino.h>

/** FlashOnly = no SD mounted; SdFull = microSD ready (exports enabled). */
enum class StorageTier {
  FlashOnly,
  SdFull
};

enum class SdRequirement {
  NotNeeded,
  Optional,
  Required
};

class StorageManager {
public:
  void begin();
  void loop();

  StorageTier getTier() const { return _tier; }
  bool isSdReady() const { return _sdReady; }
  bool isFlashReady() const { return _flashReady; }
  /** Retry LittleFS mount if boot mount failed (e.g. first upload pending). */
  bool ensureFlashMounted();
  /** Bytes free on LittleFS (0 if flash not mounted). */
  size_t freeFlashBytes() const;

  bool readTextFile(const char* path, String& out);
  bool writeTextFile(const char* path, const String& data);
  /** Writes only to SD (returns false if no card). */
  bool writeTextFileSdOnly(const char* path, const String& data);
  bool appendTextFileSdOnly(const char* path, const String& data);
  bool appendTextFile(const char* path, const String& data);
  bool writeBinaryFile(const char* path, const uint8_t* data, size_t len);
  bool writeBinaryFileSdOnly(const char* path, const uint8_t* data, size_t len);
  bool appendBinaryFile(const char* path, const uint8_t* data, size_t len);
  bool appendBinaryFileSdOnly(const char* path, const uint8_t* data, size_t len);
  bool readBinaryFile(const char* path, uint8_t* out, size_t maxLen, size_t& readLen) const;
  size_t fileSize(const char* path) const;
  bool exists(const char* path);
  bool ensureDirectory(const char* path);
  bool checkRequirement(SdRequirement requirement, String& message) const;
  /** False when no SD — fills error for export / SD-only writes. */
  bool canExportToSd(char* error, size_t errorLen) const;
  const char* getStatusLine() const;
  const char* getTierLabel() const;

  void ensureSdFolders();
  void ensureFlashFolders();

private:
  bool mountSd();
  bool mountFlash();
  bool readFromSd(const char* path, String& out);
  bool readFromFlash(const char* path, String& out);
  bool writeToSd(const char* path, const String& data);
  bool writeToFlash(const char* path, const String& data);

  StorageTier _tier = StorageTier::FlashOnly;
  bool _sdReady = false;
  bool _flashReady = false;
  bool _sdFoldersReady = false;
  bool _sdScanActive = true;
  uint32_t _sdNextTryMs = 0;
  uint32_t _sdScanStartMs = 0;
  char _statusLine[96] = "Storage: initializing";
};

extern StorageManager Storage;
