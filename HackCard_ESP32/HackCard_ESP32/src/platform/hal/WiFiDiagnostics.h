#pragma once

#include <Arduino.h>

// Detailed Wi-Fi diagnostics for hardware bring-up.
void WifiDiagPrintBootReport();
void WifiDiagPrintRuntimeReport();
const char* WifiDiagEspErrToString(int err);
