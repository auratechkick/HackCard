#pragma once

// Feature flags for the Arduino library build.
// Full firmware uses config/app_registry.h instead.
// RP2350 has no Wi-Fi and no BLE.
//
// Permanent Type 2 lock is the NFC_Kill_Type2 example.
// That sketch asks for a PIN, shows the UID, then waits for YES.

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
#define APP_NFC_CLONE             0
#endif
#ifndef APP_NFC_KILL
#define APP_NFC_KILL              1
#endif
#ifndef APP_NFC_CLASSIC
#define APP_NFC_CLASSIC           0
#endif

#ifndef APP_WIFI
#define APP_WIFI                  0
#endif
#ifndef APP_BLE
#define APP_BLE                   0
#endif
