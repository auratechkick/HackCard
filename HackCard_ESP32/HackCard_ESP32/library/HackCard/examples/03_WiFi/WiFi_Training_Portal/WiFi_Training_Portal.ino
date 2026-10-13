/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Training portal
 * FULL FIRMWARE: /wifi/training
 *
 * WHAT IT DOES:
 * Phishing-awareness style portal templates for classroom use.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Follow Serial/AP instructions after upload.
 *
 * LIBRARIES: HackCard, WiFi, WebServer
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPI.h>
#include <SD.h>

static const char* kLabPin = "1234";
static const char* kSsid = "Training-WiFi";
static const char* kPass = "training1";
static const char* kHeadline = "Sign in to continue";

DNSServer dns;
WebServer server(80);
static bool gSd = false;
static uint16_t gCaptures = 0;

String landing() {
  String h = F("<!DOCTYPE html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
               "<title>Training</title><style>body{font-family:system-ui;background:#111;color:#eee;"
               "display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0}"
               "form{background:#1b1b1b;padding:24px;border-radius:12px;width:300px}"
               "input{width:100%;margin:8px 0;padding:10px;border-radius:8px;border:1px solid #333;background:#0d0d0d;color:#eee}"
               "button{width:100%;padding:12px;border:0;border-radius:8px;background:#3b82f6;color:#fff;font-weight:600}"
               ".note{opacity:.55;font-size:.8rem;margin-top:12px}</style></head><body><form method=POST action=/submit><h2>");
  h += kHeadline;
  h += F("</h2><input name=user placeholder='Email or username'>"
         "<input name=pass type=password placeholder='Password'>"
         "<button type=submit>Sign in</button>"
         "<p class=note>TRAINING DEMO — credentials are not sent to the internet.</p>"
         "</form></body></html>");
  return h;
}

void handleSubmit() {
  String user = server.hasArg("user") ? server.arg("user") : "";
  String pass = server.hasArg("pass") ? server.arg("pass") : "";
  gCaptures++;
  Serial.printf("[capture #%u] user=\"%s\" pass_len=%u\n",
                gCaptures, user.c_str(), (unsigned)pass.length());
  if (gSd) {
    File f = SD.open("/wifi_training_log.txt", FILE_APPEND);
    if (f) {
      f.printf("%lu,%s,%u\n", millis(), user.c_str(), (unsigned)pass.length());
      f.close();
    }
  }
  server.send(200, "text/html",
              "<html><body style='font-family:system-ui;background:#111;color:#eee;padding:40px'>"
              "<h2>Training complete</h2><p>You submitted a form on a lab portal. "
              "Never enter real passwords on unknown Wi-Fi.</p>"
              "<p><a href='/portal/reveal' style='color:#93c5fd'>Why this mattered</a></p></body></html>");
  Buzzer.playSuccess();
}

void handleReveal() {
  server.send(200, "text/html",
              "<html><body style='font-family:system-ui;background:#0b1220;color:#e8eefc;padding:40px'>"
              "<h2>What happened?</h2><p>This HackCard training portal captured the form locally "
              "to teach phishing awareness. No data left the device.</p></body></html>");
}

void startPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kSsid, kPass);
  dns.start(53, "*", WiFi.softAPIP());
  server.on("/", []() { server.send(200, "text/html", landing()); });
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/portal/reveal", handleReveal);
  server.onNotFound([]() { server.send(200, "text/html", landing()); });
  server.begin();
  Serial.printf("Training AP %s / %s → http://%s\n", kSsid, kPass, WiFi.softAPIP().toString().c_str());
  WifiLed.setState(WifiLedState::ApActive);
  RgbRing.setPattern(RingPattern::Scanning);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Training Portal (web: /wifi/training)");
  Serial.println("Classroom awareness only — never on production networks.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  gSd = SD.begin(PIN_SD_CS);
  Serial.printf("Type PIN (%s) to start...\n", kLabPin);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static bool started = false;
  RgbRing.update();
  WifiLed.update();

  if (!started && Serial.available()) {
    String pin = Serial.readStringUntil('\n');
    pin.trim();
    if (pin != kLabPin) {
      Serial.println("PIN rejected.");
      Buzzer.playError();
      return;
    }
    startPortal();
    started = true;
    Buzzer.playBoot();
  }
  if (started) {
    dns.processNextRequest();
    server.handleClient();
  }
}
