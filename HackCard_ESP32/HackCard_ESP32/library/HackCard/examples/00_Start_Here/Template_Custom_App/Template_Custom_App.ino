/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Template - build your own app
 * FULL FIRMWARE: Starting point instead of full HackCard_ESP32.ino firmware
 *
 * WHAT IT DOES:
 * Minimal loop: ring, buzzer, BOOT, optional NFC tap and soft AP.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Fork this sketch; delete features you do not need.
 *
 * LIBRARIES: HackCard, WiFi (optional AP)
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

// ── EDIT ME ────────────────────────────────────────────
static const char* kOwnerName = "Your Name";
static const bool kUseNfc = true;
static const bool kUseWifiAp = false;
static const char* kApSsid = "HackCard-MyApp";
static const char* kApPass = "hackcard2026";
// ──────────────────────────────────────────────────────

#include <WiFi.h>

/** Called when NFC sees a tag — customize for your product logic. */
void onBadgeTap(const NfcTagInfo& tag) {
  Serial.printf("[app] Tap from %s\n", tag.uidHex);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard - Template Custom App");
  Serial.printf("Owner: %s\n", kOwnerName);
  Serial.println("Fork this sketch to build your product demo.");

  RgbRing.begin(55);
  Buzzer.begin(true);
  Buttons.begin();
  WifiLed.begin(40);
  RgbRing.setPattern(RingPattern::BootAnim);

  if (kUseNfc) {
    if (!Nfc.begin()) {
      Serial.println("NFC not found (continuing without it)");
    } else {
      Serial.printf("NFC: %s\n", Nfc.getChipInfo());
    }
  }

  if (kUseWifiAp) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(kApSsid, kApPass);
    Serial.printf("AP %s -> http://%s\n", kApSsid, WiFi.softAPIP().toString().c_str());
    WifiLed.setState(WifiLedState::ApActive);
  }

  delay(600);
  RgbRing.setPattern(RingPattern::Idle);
  Serial.println("Ready. Short BOOT = status blink. Tap NFC if enabled.");
}

// loop() - keep calling Hal*.update() here; add your app logic below
void loop() {
  RgbRing.update();
  WifiLed.update();
  Buttons.update();

  // BOOT short press: quick visual feedback (replace with your action)
  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    Serial.println("[app] BOOT short press");
    RgbRing.setPattern(RingPattern::Scanning);
    delay(400);
    RgbRing.setPattern(RingPattern::Idle);
  }

  // Non-blocking NFC poll (timeout ms keeps loop responsive)
  if (kUseNfc && Nfc.isReady()) {
    NfcTagInfo tag;
    if (Nfc.readTag(tag, 400) && tag.detected) {
      onBadgeTap(tag);
      delay(800);
      RgbRing.setPattern(RingPattern::Idle);
    }
  }
}
