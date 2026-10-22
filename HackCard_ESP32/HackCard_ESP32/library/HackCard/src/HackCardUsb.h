#pragma once

/**
 * USB HID + SD helpers (ported from HackCard RP2350 examples).
 * Include after HackCard.h in sketches that drive the host PC over USB.
 *
 * Arduino IDE: USB Mode = USB-OTG (TinyUSB), USB CDC On Boot = Enabled.
 */

#include "HalUsbHid.h"
#include "HidHost.h"
#include "DuckyRunner.h"
#include "HalSd.h"
