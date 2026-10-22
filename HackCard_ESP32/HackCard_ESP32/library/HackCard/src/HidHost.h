#pragma once

#include <Arduino.h>

enum class HackOs : uint8_t {
  Windows = 0,
  MacOS,
  Linux
};

class HidHost {
public:
  void setOs(HackOs os);
  HackOs os() const { return _os; }
  const char* osName() const;
  bool setOsFromName(const char* name);
  void cycleOs();

  void countdown(uint8_t seconds, const char* action);

  void chord(uint8_t modifier, uint8_t key);
  void chord3(uint8_t first, uint8_t second, uint8_t key);
  void typeText(const char* text);
  void typeLine(const char* text);

  void openLauncher();
  void runCommand(const char* command);
  void openUrl(const char* url);
  void openNotes();
  void openCalculator();
  void openTerminal();
  void lockScreen();
  bool unlockScreen(const char* password);
  void screenshot();
  void appSwitcher();
  void closeWindow();
  void selectAll();
  void copyClipboard();
  void pasteClipboard();

  uint8_t commandModifier() const;

private:
  HackOs _os = HackOs::Windows;
};

extern HidHost Host;
