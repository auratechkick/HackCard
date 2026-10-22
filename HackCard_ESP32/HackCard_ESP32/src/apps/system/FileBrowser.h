#pragma once

#include <Arduino.h>
#include <FS.h>

/** Lab Console file browser: list, open, and delete files on flash or microSD. */
class FileBrowser {
public:
  String statusJson(bool probeSd);
  bool listJson(const String& volume, const String& path, String& json, int& httpStatus);
  bool readJson(const String& volume, const String& path, String& json, int& httpStatus);
  bool removeFile(const String& volume, const String& path, String& json, int& httpStatus);
  bool openDownload(const String& volume, const String& path, fs::File& file, String& filename, String& error);

private:
  bool resolve(const String& volume, const String& path, bool& onSd, String& cleanPath, String& error) const;
};

extern FileBrowser FilesBrowser;
