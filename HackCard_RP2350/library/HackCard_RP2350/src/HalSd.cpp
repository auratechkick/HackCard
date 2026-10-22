#include "HalSd.h"

#include "HackCard_Pins.h"

#include <SD.h>
#include <SPI.h>

HalSd HackSd;

bool HalSd::begin() {
  _mounted = false;
  _sizeMb = 0;

  SPI1.setRX(PIN_SD_MISO);
  SPI1.setTX(PIN_SD_MOSI);
  SPI1.setSCK(PIN_SD_CLK);
  SPI1.begin();

  if (!SD.begin(PIN_SD_CS, SPI_HALF_SPEED, SPI1)) {
    Serial.println("[sd] Mount failed (card optional)");
    return false;
  }

  _mounted = true;
  _sizeMb = static_cast<uint32_t>(SD.size64() / (1024ULL * 1024ULL));
  Serial.printf("[sd] Mounted — %lu MB (SPI1 CS=%d)\n",
                static_cast<unsigned long>(_sizeMb), PIN_SD_CS);
  return true;
}

bool HalSd::ensureLabFolders() {
  if (!_mounted) {
    return false;
  }

  const char* folders[] = {"/hackcard", "/nfc", "/payloads", "/logs"};
  for (const char* folder : folders) {
    makeFolder(folder);
  }
  return true;
}

bool HalSd::makeFolder(const char* path) {
  if (!_mounted || !path) {
    return false;
  }
  if (SD.exists(path)) {
    return true;
  }
  return SD.mkdir(path);
}

bool HalSd::exists(const char* path) const {
  return _mounted && path && SD.exists(path);
}

bool HalSd::writeText(const char* path, const char* text) {
  if (!_mounted || !path || !text) {
    return false;
  }
  SD.remove(path);
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  file.print(text);
  file.close();
  return true;
}

bool HalSd::readText(const char* path, char* dest, size_t destSize) {
  if (!_mounted || !path || !dest || destSize < 2) {
    return false;
  }
  File file = SD.open(path, FILE_READ);
  if (!file) {
    return false;
  }
  const size_t n = file.read(reinterpret_cast<uint8_t*>(dest), destSize - 1);
  const bool tooBig = file.available() > 0;
  file.close();
  dest[n] = '\0';
  return !tooBig;
}

bool HalSd::appendLine(const char* path, const char* line) {
  if (!_mounted || !path || !line) {
    return false;
  }

  File file = SD.open(path, "a");
  if (!file) {
    return false;
  }

  file.println(line);
  file.close();
  return true;
}

void HalSd::printListing(const char* path, Stream& out) {
  if (!_mounted) {
    out.println("[sd] Not mounted");
    return;
  }

  File dir = SD.open(path ? path : "/");
  if (!dir) {
    out.println("[sd] Cannot open folder");
    return;
  }

  out.printf("[sd] %s\n", path ? path : "/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    out.print("  ");
    out.print(entry.name());
    if (entry.isDirectory()) {
      out.println("/");
    } else {
      out.print("  ");
      out.print(entry.size());
      out.println(" bytes");
    }
    entry.close();
  }
  dir.close();
}

void HalSd::printRootListing(Stream& out) {
  printListing("/", out);
}
