#pragma once

/**
 * HackCard RP2350 Arduino library
 *
 * Pin map and hardware drivers for the HackCard RP2350 board.
 * Include this header (not HackCard.h) so it can sit beside the ESP32 library.
 *
 * This board has no Wi-Fi and no BLE. Control is USB serial, the user
 * button, NFC, and the RGB strip.
 *
 * Examples: library/HackCard_RP2350/EXAMPLES.md
 */

#include "HackCard_Pins.h"
#include "HackCard_Features.h"

#include "HalRgbRing.h"
#include "HalBuzzer.h"
#include "HalButtons.h"
#include "HalNfc.h"
#include "HalSd.h"
#include "HalUsbHid.h"
#include "HidHost.h"
#include "DuckyRunner.h"
