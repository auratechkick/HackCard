/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Web feature map (Serial cheat sheet)
 * FULL FIRMWARE: Overview of all Lab Console pages
 *
 * WHAT IT DOES:
 * Prints every dashboard area and matching example folder on USB Serial.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Read the map, then open the folder you need in File -> Examples -> HackCard.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

static void section(const char* title) {
  Serial.println();
  Serial.println(title);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== HackCard 1.0.0 — web → Arduino examples ===");
  Serial.println("File -> Examples -> HackCard -> (folder below)");
  Serial.println("Full table: library/HackCard/EXAMPLES.md");

  section("-- 01_Basics (ring, buzzer, button, Wi-Fi LED) --");
  Serial.println("Hello_RGB_Ring, Ring_Control, LED_Ring_Demos");
  Serial.println("WiFi_Status_LED, Buzzer_Tunes, BOOT_Button");

  section("-- 02_NFC --");
  Serial.println("Read_UID, Tag_Info, Dump/Write/Erase/Clone/Kill Type2");
  Serial.println("Classic_Dump, Classic_Clone, Classic_Keyscan");

  section("-- 03_WiFi --");
  Serial.println("Scanner, Connect_STA, Access_Point, Captive_Portal");
  Serial.println("Beacon_Lab, EvilTwin_Sim, Training_Portal");
  Serial.println("Probe_Sniff, Deauth_Demo, Karma_Lab, EAPOL_Capture");

  section("-- 04_BLE --");
  Serial.println("Scanner, Advertise, Contact_Card_GATT");

  section("-- 05_USB_HID (TinyUSB) --");
  Serial.println("Type_Hello, Open_Website, Apps_And_Lock");
  Serial.println("Media_Keys, Mouse_Control, Key_Press");

  section("-- 05_Storage --");
  Serial.println("SD_Card_Info, SD_Lab_Folders");

  section("-- 06_RealLife --");
  Serial.println("Conference_Badge_Tap, WiFi_Guest_Portal");
  Serial.println("BLE_Business_Card, Desk_Status_Light");

  section("-- 07_System --");
  Serial.println("Diagnostics_Self_Test, Profile_vCard_Export");
  Serial.println("Mode_Status_Presets, Activity_Log_Demo, Factory_Reset");

  section("-- Build your own --");
  Serial.println("00_Start_Here/Template_Custom_App");
  Serial.println("Guide: HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md");

  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Success);
  Buzzer.playBoot();
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(20);
}
