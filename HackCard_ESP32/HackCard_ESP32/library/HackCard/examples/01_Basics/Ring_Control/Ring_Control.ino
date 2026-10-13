/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Ring control (interactive Serial)
 * FULL FIRMWARE: Apps / status ring controls
 *
 * WHAT IT DOES:
 * Type commands on Serial to set patterns, colors, demo IDs, pointer LED, brightness.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Type help on Serial for command list (p, c, d, @, b).
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

static void printHelp() {
  Serial.println();
  Serial.println("Ring commands:");
  Serial.println("  p idle|scan|success|error|off|boot|full|flash");
  Serial.println("  c R G B   d 1-12   @ index   b brightness");
  Serial.println();
}

static bool parsePattern(const String& name, RingPattern& out) {
  if (name.equalsIgnoreCase("idle")) { out = RingPattern::Idle; return true; }
  if (name.equalsIgnoreCase("scan")) { out = RingPattern::Scanning; return true; }
  if (name.equalsIgnoreCase("success")) { out = RingPattern::Success; return true; }
  if (name.equalsIgnoreCase("error")) { out = RingPattern::Error; return true; }
  if (name.equalsIgnoreCase("off")) { out = RingPattern::Off; return true; }
  if (name.equalsIgnoreCase("boot")) { out = RingPattern::BootAnim; return true; }
  if (name.equalsIgnoreCase("full")) { out = RingPattern::FullMode; return true; }
  if (name.equalsIgnoreCase("flash")) { out = RingPattern::FlashMode; return true; }
  return false;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard — Ring Control (web: status / apps)");
  RgbRing.begin(70);
  RgbRing.setPattern(RingPattern::Idle);
  printHelp();
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  const char cmd = line.charAt(0);
  const String arg = line.substring(1);
  String argTrim = arg;
  argTrim.trim();

  if (cmd == 'p') {
    RingPattern pat;
    if (!parsePattern(argTrim, pat)) {
      Serial.println("Unknown pattern. Try: p idle");
      return;
    }
    RgbRing.stopDemo();
    RgbRing.setPattern(pat);
    Serial.printf("Pattern: %s\n", argTrim.c_str());
    return;
  }

  if (cmd == 'c') {
    int r = 0, g = 0, b = 0;
    if (sscanf(argTrim.c_str(), "%d %d %d", &r, &g, &b) != 3) {
      Serial.println("Usage: c R G B");
      return;
    }
    RgbRing.stopDemo();
    RgbRing.showColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
    Serial.printf("Color %d %d %d\n", r, g, b);
    return;
  }

  if (cmd == 'd') {
    const int id = argTrim.toInt();
    if (id < 1 || id > 12) {
      Serial.println("Usage: d 1-12");
      return;
    }
    RgbRing.startDemo((uint8_t)id, 0, 120, 255);
    Serial.printf("Demo %d (3 s)\n", id);
    uint32_t start = millis();
    while (millis() - start < 3000) {
      RgbRing.update();
      delay(10);
    }
    RgbRing.stopDemo();
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  if (cmd == '@') {
    const int idx = argTrim.toInt();
    if (idx < 0 || idx >= RGB_RING_COUNT) {
      Serial.println("Usage: @ 0-11");
      return;
    }
    RgbRing.setPointer((uint8_t)idx);
    RgbRing.setPattern(RingPattern::Pointer);
    Serial.printf("Pointer LED %d\n", idx);
    return;
  }

  if (cmd == 'b') {
    const int br = argTrim.toInt();
    if (br < 10 || br > 100) {
      Serial.println("Usage: b 10-100");
      return;
    }
    RgbRing.setBrightness((uint8_t)br);
    Serial.printf("Brightness %d\n", br);
    return;
  }

  if (line.equalsIgnoreCase("help") || line == "?") {
    printHelp();
    return;
  }

  Serial.println("Unknown command. Type help");
}
