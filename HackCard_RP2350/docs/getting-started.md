# Getting started

## 1. Board package

Arduino IDE → **Boards Manager** → install **Raspberry Pi Pico/RP2040/RP2350** by Earle F. Philhower, III.

Libraries (**Library Manager**):

| Library | Used for |
|---------|----------|
| Adafruit PN532 | NFC |
| Adafruit BusIO | required by PN532 |
| Adafruit NeoPixel | RGB strip |

Keyboard, Mouse, SD, SPI, and Wire come with the core.

## 2. Install this library

```powershell
cd HackCard_RP2350\scripts
.\install-library.ps1
```

(From the repository root, or use the full path to your clone of this repo.)

Restart Arduino IDE after the copy.

## 3. Board settings

Open `HackCard_RP2350/HackCard_RP2350.ino`.

| Tools menu | Value |
|------------|--------|
| Board | **Generic RP2350** |
| Flash Size | **2MB (no FS)** |
| USB Stack | **Pico SDK** |
| CPU Speed | 150 MHz (default is fine) |

The HackCard flash chip is a W25Q16 (2MB). Do not pick a 4MB Pico 2 profile. USB Stack must stay on **Pico SDK**. Adafruit TinyUSB disables the core Keyboard and Mouse libraries.

## 4. Your contact card

Edit `HackCard_RP2350/config/user_config.h` (name, email, website), then upload.

## 5. Serial lab

1. Connect USB-C.
2. Select the COM port.
3. Upload.
4. Serial Monitor at **115200**.
5. Type `help`.

There is no `192.168.4.1` page. This board has no radio.

## Button

| Gesture | Firmware action |
|---------|-----------------|
| Short | Read one NFC tag |
| Long | Success tone and green ring |
| Double | LEDs off |

BOOT and RUN are hardware buttons. Firmware cannot test them as GPIO.

## Clear saved card data

Profile text and the lab PIN live in `config/user_config.h`, not in a settings chip. Saved tag lists, NFC dumps, Ducky scripts, and logs live on the microSD card.

To remove those files:

1. **File → Examples → HackCard RP2350 → 06_System → Factory_Reset**
2. Upload. Serial Monitor **115200**, line ending Newline.
3. Hold the **user button** for 3 seconds, or type `RESET`.
4. Upload `HackCard_RP2350.ino` again.

This does not format the whole card. It removes `/nfc`, `/ducky`, `/payloads`, `/logs`, and `/hackcard`.
