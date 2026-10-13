#pragma once

#include <Arduino.h>

struct WebUiEmbeddedFile {
  const char* uri;
  const char* mime;
  const uint8_t* data;
  size_t len;
};

extern const WebUiEmbeddedFile kWebUiFiles[];
extern const size_t kWebUiFileCount;