#include "SerialLab.h"

#include <HackCardRP2350.h>

#include "../config/user_config.h"

namespace {

constexpr size_t kLineMax = 160;
constexpr size_t kLogLines = 24;

char gLine[kLineMax];
size_t gLineLen = 0;
bool gWatch = false;
char gLog[kLogLines][96];
uint8_t gLogCount = 0;
uint8_t gLogNext = 0;

void remember(const char* line) {
#if ACTIVITY_LOGGING
  if (!line) {
    return;
  }
  strncpy(gLog[gLogNext], line, sizeof(gLog[0]) - 1);
  gLog[gLogNext][sizeof(gLog[0]) - 1] = '\0';
  gLogNext = (gLogNext + 1) % kLogLines;
  if (gLogCount < kLogLines) {
    gLogCount++;
  }
  if (HackSd.isMounted()) {
    HackSd.appendLine("/logs/activity.log", line);
  }
#else
  (void)line;
#endif
}

void printHelp() {
  Serial.println();
  Serial.println("HackCard RP2350 serial lab");
  Serial.println("  help");
  Serial.println("  status");
  Serial.println("  rgb idle|scan|ok|err|off");
  Serial.println("  beep");
  Serial.println("  nfc");
  Serial.println("  watch          toggle continuous NFC read");
  Serial.println("  url <https://...>     write NDEF URL (tag you own)");
  Serial.println("  text <message>        write NDEF text (tag you own)");
  Serial.println("  vcard                 write contact from user_config.h");
  Serial.println("  sd");
  Serial.println("  hid <text>     type into the focused window after 3 seconds");
  Serial.println("  os             show Windows / macOS / Linux");
  Serial.println("  os win|mac|linux");
  Serial.println("  site <url>     open that page on this PC");
  Serial.println("  notes          open Notepad, TextEdit, or gedit");
  Serial.println("  lock           lock this PC");
  Serial.println("  shot           screenshot on this PC");
  Serial.println("  log");
  Serial.println();
  Serial.println("Button: short = NFC read, long = success blink, double = LED off");
  Serial.println("This board has no Wi-Fi and no BLE.");
}

void printStatus() {
  Serial.printf("Board: %s  firmware %s\n", HACKCARD_BOARD_NAME, "0.1.0");
  Serial.printf("CPU %lu MHz  heap %u\n", rp2040.f_cpu() / 1000000UL, rp2040.getFreeHeap());
  Serial.printf("NFC: %s\n", Nfc.isReady() ? Nfc.getChipInfo() : "not ready");
  Serial.printf("SD: %s", HackSd.isMounted() ? "mounted" : "not mounted");
  if (HackSd.isMounted()) {
    Serial.printf(" %lu MB", static_cast<unsigned long>(HackSd.sizeMb()));
  }
  Serial.println();
  Serial.printf("Owner: %s <%s>\n", USER_NAME, USER_EMAIL);
  Serial.printf("HID OS: %s\n", Host.osName());
}

void readOnce() {
  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 400) || !tag.detected) {
    if (!gWatch) {
      Serial.println("No tag");
    }
    return;
  }

  char line[96];
  snprintf(line, sizeof(line), "NFC %s %s", tag.uidHex, tag.tagType);
  Serial.println(line);
  remember(line);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

bool writeAndReport(bool ok, const char* err, const char* success) {
  if (!ok) {
    if (err && err[0]) {
      Serial.printf("Write failed: %s\n", err);
      remember(err);
    }
    Buzzer.playError();
    RgbRing.setPattern(RingPattern::Error);
    return false;
  }
  Serial.println(success);
  remember(success);
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  return true;
}

void writeUrl(const char* url) {
  char err[64] = {0};
  writeAndReport(Nfc.writeNdefUri(url, err, sizeof(err)), err, "URL written");
}

void writeText(const char* text) {
  char err[64] = {0};
  writeAndReport(Nfc.writeNdefText(text, err, sizeof(err)), err, "Text written");
}

void writeVcard() {
  char vcard[420];
  snprintf(vcard, sizeof(vcard),
           "BEGIN:VCARD\r\n"
           "VERSION:3.0\r\n"
           "FN:%s\r\n"
           "ORG:%s\r\n"
           "TITLE:%s\r\n"
           "TEL:%s\r\n"
           "EMAIL:%s\r\n"
           "URL:%s\r\n"
           "END:VCARD\r\n",
           USER_NAME, USER_COMPANY, USER_TITLE, USER_PHONE, USER_EMAIL, USER_WEBSITE);

  char err[64] = {0};
  writeAndReport(Nfc.writeNdefVcard(vcard, err, sizeof(err)), err, "vCard written");
}

void typeOnThisPc(const char* text) {
  Serial.println("Focus a text editor on this PC. Typing in 3 seconds...");
  delay(3000);
  UsbHid.typeText(text);
  UsbHid.tapEnter();
  Serial.println("Typed.");
  remember("HID typed a line on this PC");
}

void handleLine(char* line) {
  while (*line == ' ') {
    line++;
  }
  if (*line == '\0') {
    return;
  }

  if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0) {
    printHelp();
  } else if (strcmp(line, "status") == 0) {
    printStatus();
  } else if (strcmp(line, "beep") == 0) {
    Buzzer.playTune(BuzzerTuneId::Chime);
  } else if (strcmp(line, "nfc") == 0) {
    readOnce();
  } else if (strcmp(line, "watch") == 0) {
    gWatch = !gWatch;
    RgbRing.setPattern(gWatch ? RingPattern::Scanning : RingPattern::Idle);
    Serial.println(gWatch ? "NFC watch on" : "NFC watch off");
  } else if (strcmp(line, "vcard") == 0) {
    writeVcard();
  } else if (strcmp(line, "sd") == 0) {
    if (!HackSd.isMounted() && !HackSd.begin()) {
      Serial.println("SD not available");
      return;
    }
    HackSd.ensureLabFolders();
    HackSd.printRootListing(Serial);
  } else if (strcmp(line, "log") == 0) {
    if (gLogCount == 0) {
      Serial.println("(empty)");
      return;
    }
    const uint8_t start = (gLogNext + kLogLines - gLogCount) % kLogLines;
    for (uint8_t i = 0; i < gLogCount; i++) {
      Serial.println(gLog[(start + i) % kLogLines]);
    }
  } else if (strncmp(line, "rgb ", 4) == 0) {
    const char* mode = line + 4;
    if (strcmp(mode, "idle") == 0) RgbRing.setPattern(RingPattern::Idle);
    else if (strcmp(mode, "scan") == 0) RgbRing.setPattern(RingPattern::Scanning);
    else if (strcmp(mode, "ok") == 0) RgbRing.setPattern(RingPattern::Success);
    else if (strcmp(mode, "err") == 0) RgbRing.setPattern(RingPattern::Error);
    else if (strcmp(mode, "off") == 0) RgbRing.setPattern(RingPattern::Off);
    else Serial.println("Use idle, scan, ok, err, or off");
  } else if (strncmp(line, "url ", 4) == 0) {
    writeUrl(line + 4);
  } else if (strncmp(line, "text ", 5) == 0) {
    writeText(line + 5);
  } else if (strncmp(line, "hid ", 4) == 0) {
    typeOnThisPc(line + 4);
  } else if (strcmp(line, "os") == 0) {
    Serial.printf("OS: %s\n", Host.osName());
    Serial.println("Set with: os win | os mac | os linux");
  } else if (strncmp(line, "os ", 3) == 0) {
    if (Host.setOsFromName(line + 3)) {
      Serial.printf("OS set to %s\n", Host.osName());
    } else {
      Serial.println("Use win, mac, or linux");
    }
  } else if (strncmp(line, "site ", 5) == 0) {
    Host.countdown(3, "Open site");
    Host.openUrl(line + 5);
    remember("HID opened a site on this PC");
  } else if (strcmp(line, "notes") == 0) {
    Host.countdown(3, "Open notes");
    Host.openNotes();
  } else if (strcmp(line, "lock") == 0) {
    Host.countdown(3, "Lock screen");
    Host.lockScreen();
  } else if (strcmp(line, "shot") == 0) {
    Host.countdown(2, "Screenshot");
    Host.screenshot();
  } else {
    Serial.println("Unknown command. Type help.");
  }
}

void pollButton() {
  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      readOnce();
      break;
    case ButtonGesture::Long:
      Buzzer.playSuccess();
      RgbRing.setPattern(RingPattern::Success);
      break;
    case ButtonGesture::Double:
      RgbRing.setPattern(RingPattern::Off);
      Serial.println("LEDs off");
      break;
    default:
      break;
  }
}

}  // namespace

void serialLabBegin() {
  Serial.println();
  Serial.printf("%s  firmware 0.1.0\n", HACKCARD_BOARD_NAME);
  Serial.println("No Wi-Fi on this board. Type help.");
  printStatus();
  remember("boot");
}

void serialLabUpdate() {
  RgbRing.update();
  Buttons.update();
  pollButton();

  if (gWatch) {
    readOnce();
  }

  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      gLine[gLineLen] = '\0';
      handleLine(gLine);
      gLineLen = 0;
      continue;
    }
    if (gLineLen + 1 < kLineMax) {
      gLine[gLineLen++] = c;
    }
  }
}
