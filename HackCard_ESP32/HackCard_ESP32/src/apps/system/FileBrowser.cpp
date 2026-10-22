#include "FileBrowser.h"

#include "../../core/StorageManager.h"

#include <LittleFS.h>
#include <SD.h>
#include <strings.h>

FileBrowser FilesBrowser;

namespace {

constexpr size_t kMaxPreviewBytes = 4096;
constexpr int kMaxEntries = 48;
constexpr size_t kMaxPathLen = 96;

struct ListedFile {
  char name[64];
  uint32_t size;
  bool dir;
};

void appendJsonEscaped(String& out, const char* text, size_t length) {
  out += '"';
  for (size_t i = 0; i < length; i++) {
    const unsigned char c = static_cast<unsigned char>(text[i]);
    if (c == '"' || c == '\\') {
      out += '\\';
      out += static_cast<char>(c);
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else if (c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", c);
      out += buf;
    } else {
      out += static_cast<char>(c);
    }
  }
  out += '"';
}

void appendJsonEscaped(String& out, const String& text) {
  appendJsonEscaped(out, text.c_str(), text.length());
}

String u64String(uint64_t value) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(value));
  return String(buf);
}

bool isSafePath(const String& path) {
  if (path.length() == 0 || path.length() > kMaxPathLen || path.charAt(0) != '/') {
    return false;
  }
  if (path.indexOf("..") >= 0 || path.indexOf('\\') >= 0 || path.indexOf("//") >= 0) {
    return false;
  }
  for (size_t i = 0; i < path.length(); i++) {
    const char c = path.charAt(i);
    const bool ok = c == '/' || c == '.' || c == '_' || c == '-' ||
                    (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    if (!ok) {
      return false;
    }
  }
  return true;
}

String cleanPath(String path) {
  path.trim();
  if (path.length() == 0) {
    return "/";
  }
  if (!path.startsWith("/")) {
    path = "/" + path;
  }
  if (path.length() > 1 && path.endsWith("/")) {
    path.remove(path.length() - 1);
  }
  return path;
}

bool isUiPath(const String& path) {
  return path == "/www" || path.startsWith("/www/");
}

bool isConfigPath(const String& path) {
  return path == "/config" || path.startsWith("/config/");
}

const char* cardTypeName(uint8_t type) {
  if (type == CARD_MMC) {
    return "MMC";
  }
  if (type == CARD_SD) {
    return "SD";
  }
  if (type == CARD_SDHC) {
    return "SDHC";
  }
  return "unknown";
}

bool textExtension(const char* name) {
  const char* dot = name ? strrchr(name, '.') : nullptr;
  if (!dot) {
    return false;
  }
  return strcasecmp(dot, ".txt") == 0 || strcasecmp(dot, ".json") == 0 || strcasecmp(dot, ".csv") == 0 ||
         strcasecmp(dot, ".log") == 0 || strcasecmp(dot, ".duck") == 0 || strcasecmp(dot, ".ds") == 0 ||
         strcasecmp(dot, ".vcf") == 0 || strcasecmp(dot, ".md") == 0 || strcasecmp(dot, ".xml") == 0 ||
         strcasecmp(dot, ".html") == 0;
}

bool bufferLooksBinary(const uint8_t* data, size_t length) {
  if (!data || length == 0) {
    return false;
  }
  const size_t n = length < 256 ? length : 256;
  for (size_t i = 0; i < n; i++) {
    if (data[i] == 0) {
      return true;
    }
  }
  return false;
}

int compareListed(const void* a, const void* b) {
  const ListedFile* left = static_cast<const ListedFile*>(a);
  const ListedFile* right = static_cast<const ListedFile*>(b);
  if (left->dir != right->dir) {
    return left->dir ? -1 : 1;
  }
  return strcasecmp(left->name, right->name);
}

String childPath(const String& parent, const char* name) {
  if (parent == "/") {
    return String("/") + name;
  }
  return parent + "/" + name;
}

fs::File openPath(bool onSd, const String& path, const char* mode) {
  if (onSd) {
    return SD.open(path.c_str(), mode);
  }
  return LittleFS.open(path.c_str(), mode);
}

}  // namespace

bool FileBrowser::resolve(const String& volume, const String& path, bool& onSd, String& normalized,
                          String& error) const {
  if (volume == "sd") {
    onSd = true;
  } else if (volume == "flash") {
    onSd = false;
  } else {
    error = "Volume must be sd or flash";
    return false;
  }

  normalized = cleanPath(path);
  if (!isSafePath(normalized)) {
    error = "Path must stay on this device and use letters, numbers, dot, dash, or underscore";
    return false;
  }
  if (isUiPath(normalized) || isConfigPath(normalized)) {
    error = "Setup and firmware UI files are not listed here";
    return false;
  }
  if (onSd && !Storage.isSdReady()) {
    error = "No SD card detected. Insert a FAT32 card and tap Check SD card.";
    return false;
  }
  if (!onSd && !Storage.isFlashReady()) {
    error = "On-chip flash storage is not mounted";
    return false;
  }
  return true;
}

String FileBrowser::statusJson(bool probeSd) {
  if (probeSd) {
    Storage.probeSd();
  }

  static bool usageCached = false;
  static uint64_t cachedTotal = 0;
  static uint64_t cachedUsed = 0;
  if (!Storage.isSdReady()) {
    usageCached = false;
  } else if (probeSd) {
    cachedTotal = SD.totalBytes();
    cachedUsed = SD.usedBytes();
    usageCached = true;
  }

  String json = "{\"success\":true";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"flash_ready\":";
  json += Storage.isFlashReady() ? "true" : "false";
  json += ",\"usage_known\":";
  json += usageCached ? "true" : "false";
  json += ",\"storage_status\":";
  appendJsonEscaped(json, Storage.getStatusLine());

  json += ",\"sd\":{";
  if (Storage.isSdReady()) {
    const uint64_t capacity = SD.cardSize();
    const uint64_t freeBytes = usageCached && cachedTotal > cachedUsed ? cachedTotal - cachedUsed : 0;
    json += "\"ready\":true,\"usage_known\":";
    json += usageCached ? "true" : "false";
    json += ",\"type\":\"";
    json += cardTypeName(SD.cardType());
    json += "\",\"size_bytes\":";
    json += u64String(capacity);
    json += ",\"total_bytes\":";
    json += u64String(usageCached ? cachedTotal : capacity);
    json += ",\"used_bytes\":";
    json += u64String(usageCached ? cachedUsed : 0);
    json += ",\"free_bytes\":";
    json += u64String(freeBytes);
  } else {
    json += "\"ready\":false,\"usage_known\":false,\"type\":\"none\",\"size_bytes\":0,\"total_bytes\":0,\"used_bytes\":0,\"free_bytes\":0";
  }
  json += "}";

  json += ",\"flash\":{";
  if (Storage.isFlashReady()) {
    const uint64_t total = LittleFS.totalBytes();
    const uint64_t used = LittleFS.usedBytes();
    const uint64_t freeBytes = total > used ? total - used : 0;
    json += "\"ready\":true,\"total_bytes\":";
    json += u64String(total);
    json += ",\"used_bytes\":";
    json += u64String(used);
    json += ",\"free_bytes\":";
    json += u64String(freeBytes);
  } else {
    json += "\"ready\":false,\"total_bytes\":0,\"used_bytes\":0,\"free_bytes\":0";
  }
  json += "}}";
  return json;
}

bool FileBrowser::listJson(const String& volume, const String& path, String& json, int& httpStatus) {
  bool onSd = false;
  String normalized;
  String error;
  if (!resolve(volume, path, onSd, normalized, error)) {
    httpStatus = 400;
    if ((volume == "sd" && !Storage.isSdReady()) || (volume == "flash" && !Storage.isFlashReady())) {
      httpStatus = 404;
    } else if (isUiPath(cleanPath(path)) || isConfigPath(cleanPath(path))) {
      httpStatus = 403;
    }
    json = "{\"success\":false,\"error\":";
    appendJsonEscaped(json, error);
    json += "}";
    return false;
  }

  fs::File dir = openPath(onSd, normalized, FILE_READ);
  if (!dir || !dir.isDirectory()) {
    if (dir) {
      dir.close();
    }
    httpStatus = 404;
    json = "{\"success\":false,\"error\":\"Folder not found\"}";
    return false;
  }

  ListedFile rows[kMaxEntries];
  int count = 0;
  bool truncated = false;
  fs::File entry = dir.openNextFile();
  while (entry) {
    const char* rawName = entry.name();
    const char* slash = rawName ? strrchr(rawName, '/') : nullptr;
    const char* base = slash ? slash + 1 : rawName;
    const bool isDir = entry.isDirectory();
    const uint32_t size = isDir ? 0 : static_cast<uint32_t>(entry.size());

    if (base && base[0] && strcmp(base, ".") != 0 && strcmp(base, "..") != 0) {
      if (normalized == "/" && (strcmp(base, "www") == 0 || strcmp(base, "config") == 0)) {
        // Setup and the firmware UI stay off this page.
      } else if (strlen(base) >= sizeof(rows[0].name) || childPath(normalized, base).length() > kMaxPathLen) {
        truncated = true;
      } else if (count >= kMaxEntries) {
        truncated = true;
      } else {
        strlcpy(rows[count].name, base, sizeof(rows[count].name));
        rows[count].size = size;
        rows[count].dir = isDir;
        count++;
      }
    }
    entry = dir.openNextFile();
  }
  dir.close();

  qsort(rows, static_cast<size_t>(count), sizeof(ListedFile), compareListed);

  json = "{\"success\":true,\"volume\":\"";
  json += onSd ? "sd" : "flash";
  json += "\",\"path\":";
  appendJsonEscaped(json, normalized);
  json += ",\"truncated\":";
  json += truncated ? "true" : "false";
  json += ",\"entries\":[";
  for (int i = 0; i < count; i++) {
    if (i > 0) {
      json += ',';
    }
    const String full = childPath(normalized, rows[i].name);
    json += "{\"name\":";
    appendJsonEscaped(json, rows[i].name, strlen(rows[i].name));
    json += ",\"path\":";
    appendJsonEscaped(json, full);
    json += ",\"dir\":";
    json += rows[i].dir ? "true" : "false";
    json += ",\"size\":";
    json += String(rows[i].size);
    json += '}';
  }
  json += "]}";
  httpStatus = 200;
  return true;
}

bool FileBrowser::readJson(const String& volume, const String& path, String& json, int& httpStatus) {
  bool onSd = false;
  String normalized;
  String error;
  if (!resolve(volume, path, onSd, normalized, error)) {
    httpStatus = 400;
    json = "{\"success\":false,\"error\":";
    appendJsonEscaped(json, error);
    json += "}";
    return false;
  }

  fs::File file = openPath(onSd, normalized, FILE_READ);
  if (!file || file.isDirectory()) {
    if (file) {
      file.close();
    }
    httpStatus = 404;
    json = "{\"success\":false,\"error\":\"File not found\"}";
    return false;
  }

  const size_t size = file.size();
  const size_t toRead = size < kMaxPreviewBytes ? size : kMaxPreviewBytes;
  uint8_t* buffer = nullptr;
  if (toRead > 0) {
    buffer = static_cast<uint8_t*>(malloc(toRead));
  }
  if (!buffer && toRead > 0) {
    file.close();
    httpStatus = 500;
    json = "{\"success\":false,\"error\":\"Not enough memory to open this file\"}";
    return false;
  }
  size_t got = 0;
  if (toRead > 0) {
    got = file.read(buffer, toRead);
  }
  file.close();

  const char* base = normalized.c_str();
  const char* slash = strrchr(base, '/');
  if (slash) {
    base = slash + 1;
  }
  const bool asBinary = !textExtension(base) || bufferLooksBinary(buffer, got);

  json = "{\"success\":true,\"volume\":\"";
  json += onSd ? "sd" : "flash";
  json += "\",\"path\":";
  appendJsonEscaped(json, normalized);
  json += ",\"size\":";
  json += String(static_cast<unsigned>(size));
  json += ",\"binary\":";
  json += asBinary ? "true" : "false";
  json += ",\"truncated\":";
  json += (size > got) ? "true" : "false";
  json += ",\"content\":";
  if (asBinary) {
    json += "\"\"";
  } else {
    appendJsonEscaped(json, reinterpret_cast<const char*>(buffer), got);
  }
  json += '}';
  free(buffer);
  httpStatus = 200;
  return true;
}

bool FileBrowser::removeFile(const String& volume, const String& path, String& json, int& httpStatus) {
  bool onSd = false;
  String normalized;
  String error;
  if (!resolve(volume, path, onSd, normalized, error)) {
    httpStatus = 400;
    json = "{\"success\":false,\"error\":";
    appendJsonEscaped(json, error);
    json += "}";
    return false;
  }
  if (normalized == "/" || isUiPath(normalized) || isConfigPath(normalized)) {
    httpStatus = 403;
    json = "{\"success\":false,\"error\":\"Setup, config, and firmware UI files stay on the device\"}";
    return false;
  }

  fs::File file = openPath(onSd, normalized, FILE_READ);
  if (!file) {
    httpStatus = 404;
    json = "{\"success\":false,\"error\":\"File not found\"}";
    return false;
  }
  const bool isDir = file.isDirectory();
  file.close();
  if (isDir) {
    httpStatus = 400;
    json = "{\"success\":false,\"error\":\"Open the folder and delete the files inside it\"}";
    return false;
  }

  const bool removed = onSd ? SD.remove(normalized.c_str()) : LittleFS.remove(normalized.c_str());
  if (!removed) {
    httpStatus = 500;
    json = "{\"success\":false,\"error\":\"Could not delete that file\"}";
    return false;
  }

  json = "{\"success\":true,\"volume\":\"";
  json += onSd ? "sd" : "flash";
  json += "\",\"path\":";
  appendJsonEscaped(json, normalized);
  json += '}';
  httpStatus = 200;
  return true;
}

bool FileBrowser::openDownload(const String& volume, const String& path, fs::File& file, String& filename,
                               String& error) {
  bool onSd = false;
  String normalized;
  if (!resolve(volume, path, onSd, normalized, error)) {
    return false;
  }
  file = openPath(onSd, normalized, FILE_READ);
  if (!file || file.isDirectory()) {
    if (file) {
      file.close();
    }
    error = "File not found";
    return false;
  }
  const int slash = normalized.lastIndexOf('/');
  filename = slash >= 0 ? normalized.substring(slash + 1) : normalized;
  filename.replace("\"", "");
  return true;
}
