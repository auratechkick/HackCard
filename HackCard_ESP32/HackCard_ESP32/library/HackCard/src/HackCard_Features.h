#pragma once

// Feature flags for the Arduino library build.
// Full firmware uses config/app_registry.h instead.

#ifndef APP_NFC_READ
#define APP_NFC_READ              1
#endif
#ifndef APP_NFC_INFO
#define APP_NFC_INFO              1
#endif
#ifndef APP_NFC_DUMP
#define APP_NFC_DUMP              1
#endif
#ifndef APP_NFC_WRITE
#define APP_NFC_WRITE             1
#endif
#ifndef APP_NFC_CLONE
#define APP_NFC_CLONE             1
#endif
#ifndef APP_NFC_KILL
#define APP_NFC_KILL              1
#endif
#ifndef APP_NFC_CLASSIC
#define APP_NFC_CLASSIC           1
#endif
