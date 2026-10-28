# Install tools & connect USB

Prepare Arduino for **Generic RP2350** and connect HackCard over USB-C.

---

## Step 1 — Connect HackCard

1. USB-C **data** cable to your PC.
2. Confirm a **COM port** appears after drivers install (Windows may need a moment for Pico/RP2350 CDC).

---

## Step 2 — Install Arduino IDE

**[Arduino IDE 2.x](https://www.arduino.cc/en/software)**

---

## Step 3 — Install RP2350 board support

**Tools → Board → Boards Manager** → install:

**Raspberry Pi Pico/RP2040/RP2350** by **Earle F. Philhower, III** (`arduino-pico`).

---

## Step 4 — Install libraries

**Tools → Manage Libraries:**

| Library | Purpose |
|---------|---------|
| Adafruit PN532 | NFC |
| Adafruit BusIO | PN532 dependency |
| Adafruit NeoPixel | RGB strip |

Keyboard, Mouse, SD, SPI, Wire ship with the Philhower core.

---

## Step 5 — Install HackCard_RP2350 library

From repo root:

```powershell
cd HackCard_RP2350\scripts
.\install-library.ps1
```

Restart Arduino IDE. Examples appear under **File → Examples → HackCard RP2350**.

---

## Step 6 — Open firmware sketch

```
HackCard_RP2350/HackCard_RP2350/HackCard_RP2350.ino
```

**Tools** (critical):

| Setting | Value |
|---------|--------|
| Board | **Generic RP2350** |
| Flash Size | **2MB (no FS)** — W25Q16 on HackCard |
| USB Stack | **Pico SDK** (not Adafruit TinyUSB) |
| CPU Speed | 150 MHz (default OK) |

Wrong flash size (e.g. 4 MB Pico 2 profile) or wrong USB stack breaks upload or HID.

Full table: [ARDUINO_SETUP.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/HackCard_RP2350/docs/ARDUINO_SETUP.md) in the firmware tree.

---

## Next step

→ [Personalize settings](personalize.md) → [Flash firmware](first-program.md)
