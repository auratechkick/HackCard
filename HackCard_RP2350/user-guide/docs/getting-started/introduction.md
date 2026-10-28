# Introduction

HackCard **RP2350** is the USB-centric sibling of the ESP32-S3 board: same NFC and HID teaching goals, **without Wi-Fi, BLE, or a web Lab Console**.

You flash firmware with Arduino IDE, then run labs from **Serial Monitor @ 115200** (`help`, `nfc`, `hid`, …) while USB HID actions go to the **PC the cable is plugged into**.

---

## What you get in software

| Piece | What it does |
|-------|----------------|
| **HackCard_RP2350.ino** | Serial lab firmware — commands, NFC, HID demos |
| **HackCard_RP2350 library** | **40** standalone Arduino examples |
| **Production test sketch** | Factory QC (`sketches/HackCard_Production_Test/`) |

---

## RP2350 vs ESP32-S3

| | **RP2350** | **ESP32-S3** |
|---|------------|--------------|
| **Control UI** | Serial @ 115200 | Lab Console @ `http://192.168.4.1/` |
| **Wi-Fi / BLE labs** | No | Yes |
| **USB HID to PC** | Yes (Pico SDK USB) | Yes (TinyUSB) |
| **NFC (PN532)** | Yes | Yes |
| **Arduino core** | Philhower **arduino-pico** | Espressif **esp32** |
| **User guide** | [RP2350 guide](https://auratechkick.github.io/HackCard/rp2350/) | [ESP32 guide](https://auratechkick.github.io/HackCard/esp32/) |

---

## What HackCard RP2350 is not

- **Not** a Wi-Fi tool — no scan, AP, or captive portal on this chip.
- **Not** an NFC **emulator** — PN532 is a **reader/writer** for external tags.
- **Not** a hidden keyboard on someone else’s PC without consent — HID only affects the **host you plug into**.

---

## Who this guide is for

| You are… | Start here |
|----------|------------|
| **First flash** | [What's on the board](whats-included.md) → [First setup](first-setup.md) |
| **Daily use** | [Serial lab console](serial-lab.md) |
| **Learning by example** | [Arduino examples](arduino-examples.md) |
| **Custom firmware** | [Write your own sketch](write-your-own-sketch.md) |

Firmware version: **`0.1.0`** (`HackCard_RP2350/config/board_config.h`).
