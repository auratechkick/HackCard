# Changelog

## 0.1.0 — factory reset example (2026-10-22)

- Example `06_System/Factory_Reset` clears HackCard folders on the microSD card after the user button is held or Serial `RESET` is typed
- Profile and lab PIN stay in `config/user_config.h` (this board has no Wi-Fi and no settings filesystem)
- Example count is **41**

## 0.1.0 — GitHub (2026-10-22)

- Published under `HackCard_RP2350/` beside `HackCard_ESP32/` in [auratechkick/HackCard](https://github.com/auratechkick/HackCard)
- Documentation hub: `docs/README.md`, `CUSTOM_DEVELOPMENT.md`, updated getting-started paths

## 0.1.0 — initial tree

- First RP2350 tree, structured like the ESP32 repo.
- Serial lab firmware instead of a Wi-Fi Lab Console.
- Library: RGB strip, buzzer, user button, PN532 on I2C1, microSD on SPI1, USB HID.
- Examples for NFC, HID, SD, desk light, badge tap, and diagnostics.
- Production-test sketch copied from the verified RP2350 QC firmware 1.0.4.
- Wi-Fi and BLE are not included. This chip has no radio.
