/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Profile vCard export
 * FULL FIRMWARE: /profile /setup
 *
 * WHAT IT DOES:
 * Builds vCard text from EDIT ME contact fields and prints to Serial.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Copy vCard from Serial to contacts app.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <SPI.h>
#include <SD.h>

static const char* kName = "Alex Rivera";
static const char* kTitle = "Security Researcher";
static const char* kOrg = "Hardware Hack Space";
static const char* kEmail = "alex@example.com";
static const char* kPhone = "+1 555 0100";
static const char* kWebsite = "https://hardwarehackspace.github.io/hackcard-docs/";
static const char* kLinkedIn = "https://linkedin.com/in/example";
static const char* kGithub = "https://github.com/hardwarehackspace";
static const char* kBio = "Ethical hacking & embedded security.";

String buildVcard() {
  String v = "BEGIN:VCARD\nVERSION:3.0\n";
  v += "FN:"; v += kName; v += "\n";
  v += "TITLE:"; v += kTitle; v += "\n";
  v += "ORG:"; v += kOrg; v += "\n";
  v += "EMAIL:"; v += kEmail; v += "\n";
  v += "TEL:"; v += kPhone; v += "\n";
  v += "URL:"; v += kWebsite; v += "\n";
  v += "NOTE:"; v += kBio; v += "\n";
  v += "X-SOCIALPROFILE;TYPE=linkedin:"; v += kLinkedIn; v += "\n";
  v += "X-SOCIALPROFILE;TYPE=github:"; v += kGithub; v += "\n";
  v += "END:VCARD\n";
  return v;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Profile vCard (web: /profile)");
  Serial.println("── Profile ──");
  Serial.printf("Name:  %s\n", kName);
  Serial.printf("Title: %s @ %s\n", kTitle, kOrg);
  Serial.printf("Email: %s\n", kEmail);
  Serial.printf("Phone: %s\n", kPhone);
  Serial.printf("Web:   %s\n", kWebsite);
  Serial.printf("Bio:   %s\n", kBio);
  Serial.println("── vCard ──");
  String v = buildVcard();
  Serial.print(v);

  RgbRing.begin(40);
  Buzzer.begin(true);
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (SD.begin(PIN_SD_CS)) {
    File f = SD.open("/profile.vcf", FILE_WRITE);
    if (f) {
      f.print(v);
      f.close();
      Serial.println("Saved SD:/profile.vcf");
    }
  } else {
    Serial.println("No SD — Serial export only (OK).");
  }
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(20);
}
