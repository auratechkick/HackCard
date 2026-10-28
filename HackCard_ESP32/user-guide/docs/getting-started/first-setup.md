# Install tools & connect USB

Prepare your computer and connect HackCard for the first firmware upload.

---

## Step 1 — Connect HackCard

1. Use a USB-C cable that supports **data** (not charge-only).
2. Plug into your PC. Windows should assign a **COM port**; macOS/Linux show a serial device.
3. If no port appears, see **Troubleshooting** in [Next steps](next-steps.md).

Power is primarily from USB (`board_config.h` — USB power path).

---

## Step 2 — Install Arduino IDE

Download **[Arduino IDE 2.x](https://www.arduino.cc/en/software)** and install it.

---

## Step 3 — Install ESP32 board support

1. **File → Preferences**
2. **Additional boards manager URLs** — add:

    ```
    https://espressif.github.io/arduino-esp32/package_esp32_index.json
    ```

3. **Tools → Board → Boards Manager** → search **esp32** → install **esp32 by Espressif Systems**

Use a recent 3.x core if available; match the monorepo’s [ARDUINO_SETUP.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/docs/ARDUINO_SETUP.md) if upload fails.

---

## Step 4 — Install libraries

**Tools → Manage Libraries** — install:

| Library | Used for |
|---------|----------|
| **Adafruit NeoPixel** | RGB ring + Wi-Fi LED |
| **Adafruit PN532** | NFC reader |
| **Adafruit BusIO** | PN532 dependency |
| **ArduinoJson** | Saved config / profile |

---

## Step 5 — Get the firmware source

Clone or download the monorepo:

**[github.com/auratechkick/HackCard](https://github.com/auratechkick/HackCard)**

Open this sketch in Arduino IDE:

```
HackCard_ESP32/HackCard_ESP32/HackCard_ESP32.ino
```

(From repo root: folder `HackCard_ESP32`, then inner `HackCard_ESP32` sketch directory.)

---

## Step 6 — Board settings (before first upload)

In **Tools**, set at minimum:

| Setting | Value |
|---------|--------|
| Board | **ESP32S3 Dev Module** |
| USB CDC On Boot | **Enabled** |
| USB Mode | **USB-OTG (TinyUSB)** — required for HID lab |
| Flash Size | **4MB (32Mb)** |
| PSRAM | **OPI PSRAM** |
| Partition Scheme | **Custom partition table** (uses sketch `partitions.csv`) |
| Upload Speed | 921600 (or lower if upload is unstable) |

Full table: [ARDUINO_SETUP.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/docs/ARDUINO_SETUP.md).

---

## Next step

→ [Personalize settings](personalize.md) (recommended before first demo)  
→ Or skip to [Flash firmware](first-program.md)
