#pragma once

#include <Arduino.h>

class HalSd {
public:
  bool begin();
  bool isMounted() const { return _mounted; }
  uint32_t sizeMb() const { return _sizeMb; }

  bool ensureLabFolders();
  bool makeFolder(const char* path);
  bool exists(const char* path) const;
  bool writeText(const char* path, const char* text);
  bool readText(const char* path, char* dest, size_t destSize);
  bool appendLine(const char* path, const char* line);
  void printListing(const char* path, Stream& out);
  void printRootListing(Stream& out);

private:
  bool _mounted = false;
  uint32_t _sizeMb = 0;
};

extern HalSd HackSd;
