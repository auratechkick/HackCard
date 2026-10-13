#pragma once

// Unified LED + buzzer feedback for NFC lab operations.

class NfcFeedback {
public:
  static void scanStart();
  static void success();
  static void error();
  static void writeOk();
  static void cloneOk();
  static void killOk();
};
