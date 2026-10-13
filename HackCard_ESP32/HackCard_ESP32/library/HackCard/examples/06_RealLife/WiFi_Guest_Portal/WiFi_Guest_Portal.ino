/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi guest portal demo
 * FULL FIRMWARE: /portal + AP
 *
 * WHAT IT DOES:
 * End-to-end guest Wi-Fi + landing page style demo.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Join AP from phone; follow Serial URL.
 *
 * LIBRARIES: HackCard, WiFi, WebServer
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <WebServer.h>

static const char* kSsid = "HackCard-Guest";
static const char* kPass = "welcome123";  // min 8 chars

// Edit for your event
static const char* kHostName = "Alex Rivera";
static const char* kEvent = "Hardware Hack Meetup";
static const char* kMessage = "Thanks for joining our guest Wi-Fi demo.";

WebServer server(80);

String landingHtml() {
  String html = F("<!DOCTYPE html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
                  "<title>Guest Wi-Fi</title><style>"
                  "body{font-family:system-ui;background:#0b1220;color:#e8eefc;display:flex;"
                  "align-items:center;justify-content:center;min-height:100vh;margin:0}"
                  ".card{max-width:420px;padding:28px;border:1px solid #2a3a5a;border-radius:16px;"
                  "background:#121a2b}"
                  "h1{margin:0 0 8px;font-size:1.4rem}p{opacity:.85;line-height:1.5}"
                  ".pill{display:inline-block;padding:4px 10px;border-radius:999px;"
                  "background:#1e3a5f;font-size:.8rem;margin-bottom:12px}"
                  "</style></head><body><div class=card>");
  html += "<div class=pill>";
  html += kEvent;
  html += "</div><h1>Welcome</h1><p>Hosted by <b>";
  html += kHostName;
  html += "</b></p><p>";
  html += kMessage;
  html += "</p><p style='opacity:.55;font-size:.85rem'>HackCard lab demo — educational use only.</p>";
  html += "</div></body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", landingHtml());
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — Wi-Fi Guest Portal");

  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(kSsid, kPass)) {
    Serial.println("softAP failed");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  server.on("/", handleRoot);
  server.onNotFound(handleRoot);
  server.begin();

  Serial.printf("AP: %s / %s\n", kSsid, kPass);
  Serial.printf("Open http://%s\n", WiFi.softAPIP().toString().c_str());

  Buzzer.playBoot();
  WifiLed.setState(WifiLedState::ApActive);
  RgbRing.setPattern(RingPattern::Idle);
}

// loop() - keep calling Hal*.update() here
void loop() {
  server.handleClient();
  WifiLed.update();
  RgbRing.update();
}
