/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Ducky Script runner
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Scripts type on the PC this board is plugged into. Run only scripts you wrote.
 *
 * Pick the OS with w, m, or l. The button cycles it.
 *   1 hello      open notes and type a greeting
 *   2 website    open kUrl
 *   3 standup    open notes and type a standup
 *   4 lock       lock this PC
 *   5 signature  type a signature into the window you focus
 *   d            list /ducky on the microSD card
 *   run name.txt run that file from /ducky
 *
 * Sample scripts are copied to /ducky the first time a card is inserted.
 * Edit those .txt files in a normal editor, then run them. Commands:
 *   REM, DELAY, DEFAULT_DELAY, STRING, STRINGLN, REPEAT
 *   GUI, ALT, CTRL, SHIFT, ENTER, arrows, F1–F12
 * GUI is the Windows key, or Command on a Mac.
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";
static const char* kUrl = "https://example.com";

static char gScript[1200];
static char gFileScript[4096];

const char* notesApp() {
  switch (Host.os()) {
    case HackOs::MacOS:
      return "TextEdit";
    case HackOs::Linux:
      return "gedit";
    default:
      return "notepad";
  }
}

const char* launcher() {
  switch (Host.os()) {
    case HackOs::MacOS:
      return "GUI SPACE";
    case HackOs::Linux:
      return "ALT F2";
    default:
      return "GUI r";
  }
}

const char* osTag() {
  switch (Host.os()) {
    case HackOs::MacOS:
      return "macos";
    case HackOs::Linux:
      return "linux";
    default:
      return "windows";
  }
}

void showOs() {
  switch (Host.os()) {
    case HackOs::MacOS:
      RgbRing.showColor(210, 210, 220);
      break;
    case HackOs::Linux:
      RgbRing.showColor(230, 110, 16);
      break;
    default:
      RgbRing.showColor(16, 80, 220);
      break;
  }
  Serial.printf("OS: %s\n", Host.osName());
}

void buildHello() {
  snprintf(gScript, sizeof(gScript),
           "REM Hello on %s\n"
           "DEFAULT_DELAY 20\n"
           "%s\n"
           "DELAY 700\n"
           "STRING %s\n"
           "ENTER\n"
           "DELAY 900\n"
           "STRINGLN Hello from HackCard. I am %s.\n",
           Host.osName(), launcher(), notesApp(), kName);
}

void buildWebsite() {
  snprintf(gScript, sizeof(gScript),
           "REM Open a page on %s\n"
           "%s\n"
           "DELAY 700\n"
           "STRING %s\n"
           "ENTER\n",
           Host.osName(), launcher(), kUrl);
}

void buildStandup() {
  snprintf(gScript, sizeof(gScript),
           "REM Standup notes on %s\n"
           "%s\n"
           "DELAY 700\n"
           "STRING %s\n"
           "ENTER\n"
           "DELAY 900\n"
           "STRINGLN Standup — %s\n"
           "STRINGLN Yesterday:\n"
           "STRINGLN - \n"
           "STRINGLN Today:\n"
           "STRINGLN - \n"
           "STRINGLN Blockers:\n"
           "STRINGLN - \n",
           Host.osName(), launcher(), notesApp(), kName);
}

void buildLock() {
  if (Host.os() == HackOs::MacOS) {
    snprintf(gScript, sizeof(gScript), "REM Lock macOS\nCTRL GUI q\n");
  } else if (Host.os() == HackOs::Linux) {
    snprintf(gScript, sizeof(gScript), "REM Lock Linux (GNOME Super+L)\nGUI l\n");
  } else {
    snprintf(gScript, sizeof(gScript), "REM Lock Windows\nGUI l\n");
  }
}

void buildSignature() {
  snprintf(gScript, sizeof(gScript),
           "REM Type into the window you already focused\n"
           "STRINGLN %s\n"
           "STRINGLN Security Researcher\n"
           "STRINGLN %s\n",
           kName, kUrl);
}

bool runCurrent(const char* title) {
  Serial.println();
  Serial.printf("Script: %s\n", title);
  Host.countdown(3, title);
  RgbRing.setPattern(RingPattern::Scanning);
  const bool ok = Ducky.run(gScript);
  if (!ok) {
    Serial.printf("Stopped: %s\n", Ducky.lastError());
    Buzzer.playError();
    RgbRing.setPattern(RingPattern::Error);
    return false;
  }
  Serial.println("Script finished.");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(400);
  showOs();
  return true;
}

void seedFile(const char* name, const char* body) {
  char path[48];
  snprintf(path, sizeof(path), "/ducky/%s", name);
  if (HackSd.exists(path)) {
    return;
  }
  if (HackSd.writeText(path, body)) {
    Serial.printf("Wrote %s\n", path);
  }
}

void seedSdScripts() {
  if (!HackSd.isMounted()) {
    return;
  }
  HackSd.makeFolder("/ducky");

  const HackOs saved = Host.os();
  const HackOs all[] = {HackOs::Windows, HackOs::MacOS, HackOs::Linux};
  for (HackOs os : all) {
    Host.setOs(os);
    char helloName[32];
    char siteName[32];
    char standupName[32];
    char lockName[32];
    snprintf(helloName, sizeof(helloName), "hello-%s.txt", osTag());
    snprintf(siteName, sizeof(siteName), "website-%s.txt", osTag());
    snprintf(standupName, sizeof(standupName), "standup-%s.txt", osTag());
    snprintf(lockName, sizeof(lockName), "lock-%s.txt", osTag());
    buildHello();
    seedFile(helloName, gScript);
    buildWebsite();
    seedFile(siteName, gScript);
    buildStandup();
    seedFile(standupName, gScript);
    buildLock();
    seedFile(lockName, gScript);
  }
  Host.setOs(saved);
  buildSignature();
  seedFile("signature.txt", gScript);
  showOs();
}

void runSdFile(const char* name) {
  char path[64];
  snprintf(path, sizeof(path), "/ducky/%s", name);
  if (!HackSd.isMounted()) {
    Serial.println("No SD card.");
    return;
  }
  if (!HackSd.readText(path, gFileScript, sizeof(gFileScript))) {
    Serial.printf("Cannot read %s (missing or larger than %u bytes)\n", path,
                  static_cast<unsigned>(sizeof(gFileScript) - 1));
    return;
  }
  strncpy(gScript, gFileScript, sizeof(gScript) - 1);
  gScript[sizeof(gScript) - 1] = '\0';
  if (strlen(gFileScript) >= sizeof(gScript)) {
    Serial.println("That file is too long for a built-in slot. Reading the SD copy directly.");
    Host.countdown(3, name);
    if (!Ducky.run(gFileScript)) {
      Serial.printf("Stopped: %s\n", Ducky.lastError());
      Buzzer.playError();
      RgbRing.setPattern(RingPattern::Error);
      return;
    }
    Serial.println("Script finished.");
    Buzzer.playSuccess();
    showOs();
    return;
  }
  runCurrent(name);
}

void printHelp() {
  Serial.println();
  Serial.println("1 hello   2 website   3 standup   4 lock   5 signature");
  Serial.println("w/m/l or short press = OS");
  Serial.println("d = list /ducky    run filename.txt = run an SD script");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Ducky Scripts");
  printHelp();

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(50);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);

  if (HackSd.begin()) {
    seedSdScripts();
    Serial.println("SD scripts are in /ducky");
  } else {
    Serial.println("No SD card. Built-in scripts still run.");
  }
  showOs();
}

void loop() {
  Buttons.update();
  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    Host.cycleOs();
    Buzzer.playTune(BuzzerTuneId::Pulse);
    showOs();
  }

  if (!Serial.available()) {
    return;
  }

  char line[64];
  size_t n = 0;
  const uint32_t started = millis();
  while (millis() - started < 40 && n + 1 < sizeof(line)) {
    while (Serial.available() && n + 1 < sizeof(line)) {
      const char c = static_cast<char>(Serial.read());
      if (c == '\r') {
        continue;
      }
      if (c == '\n') {
        line[n] = '\0';
        goto haveLine;
      }
      line[n++] = c;
    }
    delay(2);
  }
  line[n] = '\0';
  if (n == 0) {
    return;
  }

haveLine:
  if (strcmp(line, "1") == 0) {
    buildHello();
    runCurrent("hello");
  } else if (strcmp(line, "2") == 0) {
    buildWebsite();
    runCurrent("website");
  } else if (strcmp(line, "3") == 0) {
    buildStandup();
    runCurrent("standup");
  } else if (strcmp(line, "4") == 0) {
    buildLock();
    runCurrent("lock");
  } else if (strcmp(line, "5") == 0) {
    buildSignature();
    runCurrent("signature");
  } else if (strcmp(line, "d") == 0 || strcmp(line, "D") == 0) {
    if (HackSd.isMounted()) {
      HackSd.printListing("/ducky", Serial);
    } else {
      Serial.println("No SD card.");
    }
  } else if (strcmp(line, "w") == 0 || strcmp(line, "W") == 0) {
    Host.setOs(HackOs::Windows);
    showOs();
  } else if (strcmp(line, "m") == 0 || strcmp(line, "M") == 0) {
    Host.setOs(HackOs::MacOS);
    showOs();
  } else if (strcmp(line, "l") == 0 || strcmp(line, "L") == 0) {
    Host.setOs(HackOs::Linux);
    showOs();
  } else if (strncmp(line, "run ", 4) == 0) {
    runSdFile(line + 4);
  } else {
    printHelp();
  }
}
