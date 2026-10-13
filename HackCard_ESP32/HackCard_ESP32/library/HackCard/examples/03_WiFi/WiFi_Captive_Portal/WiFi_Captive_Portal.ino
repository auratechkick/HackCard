/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Captive portal
 * FULL FIRMWARE: /portal
 *
 * WHAT IT DOES:
 * Soft AP plus minimal captive landing for training demos.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Connect phone to AP; portal page opens when implemented in sketch.
 *
 * LIBRARIES: HackCard, WiFi, WebServer
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

static const char* kSsid = "HackCard-Portal";
static const char* kPass = "hackcard2026";
static const char* kName = "Alex Rivera";
static const char* kBio = "Tap to explore HackCard labs.";

DNSServer dns;
WebServer server(80);

String page() {
  String h = F("<!DOCTYPE html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
               "<title>HackCard</title><style>body{font-family:system-ui;background:#0b1220;color:#e8eefc;"
               "display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0}"
               ".c{max-width:420px;padding:28px;border:1px solid #2a3a5a;border-radius:16px;background:#121a2b}"
               "h1{margin:0 0 8px}p{opacity:.85;line-height:1.5}</style></head><body><div class=c><h1>");
  h += kName;
  h += "</h1><p>";
  h += kBio;
  h += "</p><p style='opacity:.5;font-size:.85rem'>Captive portal demo — educational use.</p></div></body></html>";
  return h;
}

void sendPage() { server.send(200, "text/html", page()); }

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Captive Portal (web: /portal)");

  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(kSsid, kPass);
  dns.start(53, "*", WiFi.softAPIP());

  server.on("/", sendPage);
  server.on("/generate_204", sendPage);
  server.on("/hotspot-detect.html", sendPage);
  server.on("/ncsi.txt", []() { server.send(200, "text/plain", "Microsoft NCSI"); });
  server.onNotFound(sendPage);
  server.begin();

  Serial.printf("AP %s / %s → http://%s\n", kSsid, kPass, WiFi.softAPIP().toString().c_str());
  Serial.println("Join from phone; captive sheet should open the landing page.");
  WifiLed.setState(WifiLedState::ApActive);
  RgbRing.setPattern(RingPattern::Idle);
  Buzzer.playBoot();
}

// loop() - keep calling Hal*.update() here
void loop() {
  dns.processNextRequest();
  server.handleClient();
  WifiLed.update();
  RgbRing.update();
}
