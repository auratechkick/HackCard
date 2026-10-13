#include "NfcHistory.h"

#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"

NfcHistory NfcHist;

namespace {

constexpr const char* kHistoryPath = "/nfc/history.csv";
constexpr const char* kHistorySdPath = "/nfc/exports/history_live.csv";
constexpr const char* kLastDumpPath = "/nfc/last_dump.txt";

void ensureNfcDir() {
  if (!Storage.exists("/nfc")) {
    Storage.writeTextFile("/nfc/.keep", "");
  }
  if (!Storage.exists("/nfc/dumps")) {
    Storage.writeTextFile("/nfc/dumps/.keep", "");
  }
}

}  // namespace

const char* NfcHistory::historyPath() const {
  return _historySdOnly ? kHistorySdPath : kHistoryPath;
}

void NfcHistory::append(const char* action, const char* uid, const char* detail) {
  ensureNfcDir();
  const char* path = historyPath();
  if (_historySdOnly) {
    Storage.ensureDirectory("/nfc/exports");
    if (!Storage.isSdReady()) {
      return;
    }
    if (!Storage.exists(path)) {
      Storage.writeTextFileSdOnly(path, "ms,action,uid,detail\n");
    }
    String line = String(millis()) + "," + action + "," + uid + "," + detail + "\n";
    Storage.appendTextFileSdOnly(path, line);
    return;
  }

  if (!Storage.exists(path)) {
    Storage.writeTextFile(path, "ms,action,uid,detail\n");
  }
  String line = String(millis()) + "," + action + "," + uid + "," + detail + "\n";
  Storage.appendTextFile(path, line);
}

void NfcHistory::setLastDumpPath(const char* path) {
  ensureNfcDir();
  if (!path || path[0] == '\0') {
    return;
  }
  Storage.writeTextFile(kLastDumpPath, path);
}

String NfcHistory::readRecent(uint16_t maxLines) {
  String raw;
  if (!Storage.readTextFile(historyPath(), raw) || raw.isEmpty()) {
    if (!Storage.readTextFile(kHistoryPath, raw) || raw.isEmpty()) {
      if (!Storage.readTextFile(kHistorySdPath, raw) || raw.isEmpty()) {
        return "No NFC history yet.\n";
      }
    }
  }

  String lines[32];
  uint16_t count = 0;
  int lineStart = 0;

  for (int i = 0; i <= raw.length(); i++) {
    if (i == raw.length() || raw[i] == '\n') {
      if (i > lineStart && count < 32) {
        lines[count++] = raw.substring(lineStart, i);
      }
      lineStart = i + 1;
    }
  }

  String out = "";
  const uint16_t start = (count > maxLines) ? (count - maxLines) : 0;
  for (uint16_t i = start; i < count; i++) {
    if (lines[i].startsWith("ms,")) {
      continue;
    }
    out += lines[i];
    out += "\n";
  }
  return out;
}

String NfcHistory::lastDumpPath() const {
  String saved;
  if (Storage.readTextFile(kLastDumpPath, saved) && saved.length() > 0) {
    saved.trim();
    if (Storage.exists(saved.c_str())) {
      return saved;
    }
  }

  String raw;
  if (!Storage.readTextFile(kHistoryPath, raw) || raw.isEmpty()) {
    return "";
  }

  String lastPath = "";
  int lineStart = 0;
  for (int i = 0; i <= raw.length(); i++) {
    if (i == raw.length() || raw[i] == '\n') {
      String line = raw.substring(lineStart, i);
      lineStart = i + 1;
      if (line.startsWith("ms,")) {
        continue;
      }
      const int c1 = line.indexOf(',');
      const int c2 = line.indexOf(',', c1 + 1);
      const int c3 = line.indexOf(',', c2 + 1);
      if (c1 < 0 || c2 < 0 || c3 < 0) {
        continue;
      }
      const String action = line.substring(c1 + 1, c2);
      if (action == "dump") {
        lastPath = line.substring(c3 + 1);
        lastPath.trim();
      }
    }
  }
  return lastPath;
}

bool nfcLabPinOk(const char* pin) {
  return labPinMatches(pin);
}

bool NfcHistory::exportPathCopyToSd(const char* sourcePath, const char* prefix, char* outPath, size_t outPathLen,
                                    char* error, size_t errorLen) const {
  if (!outPath || outPathLen == 0) {
    if (error && errorLen > 0) {
      strncpy(error, "Invalid export path buffer", errorLen - 1);
    }
    return false;
  }
  outPath[0] = '\0';

  if (!Storage.isSdReady()) {
    if (error && errorLen > 0) {
      strncpy(error, "Insert an SD card (Full Mode)", errorLen - 1);
    }
    return false;
  }
  if (!sourcePath || sourcePath[0] == '\0' || !Storage.exists(sourcePath)) {
    if (error && errorLen > 0) {
      strncpy(error, "No file to export", errorLen - 1);
    }
    return false;
  }

  String data;
  if (!Storage.readTextFile(sourcePath, data) || data.isEmpty()) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not read source file", errorLen - 1);
    }
    return false;
  }

  Storage.ensureDirectory("/nfc/exports");
  snprintf(outPath, outPathLen, "/nfc/exports/%s_%lu.txt", prefix ? prefix : "nfc",
           static_cast<unsigned long>(millis() / 1000UL));

  if (!Storage.writeTextFileSdOnly(outPath, data)) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not write export file on SD", errorLen - 1);
    }
    outPath[0] = '\0';
    return false;
  }

  Log.activity("nfc", "export_sd", outPath);
  return true;
}

bool NfcHistory::exportHistoryToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const {
  String raw;
  if (!Storage.readTextFile(historyPath(), raw) || raw.isEmpty()) {
    if (!Storage.readTextFile(kHistoryPath, raw) || raw.isEmpty()) {
      if (error && errorLen > 0) {
        strncpy(error, "No NFC history to export", errorLen - 1);
      }
      return false;
    }
  }

  if (!Storage.isSdReady()) {
    if (error && errorLen > 0) {
      strncpy(error, "Insert an SD card (Full Mode)", errorLen - 1);
    }
    return false;
  }

  Storage.ensureDirectory("/nfc/exports");
  snprintf(outPath, outPathLen, "/nfc/exports/history_%lu.csv", static_cast<unsigned long>(millis() / 1000UL));

  if (!Storage.writeTextFileSdOnly(outPath, raw)) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not write export file on SD", errorLen - 1);
    }
    if (outPath) {
      outPath[0] = '\0';
    }
    return false;
  }

  Log.activity("nfc", "history_export_sd", outPath);
  return true;
}

bool NfcHistory::exportLastDumpToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const {
  const String path = lastDumpPath();
  if (path.isEmpty()) {
    if (error && errorLen > 0) {
      strncpy(error, "No dump saved yet — run Dump Type 2 first", errorLen - 1);
    }
    return false;
  }
  return exportPathCopyToSd(path.c_str(), "dump", outPath, outPathLen, error, errorLen);
}
