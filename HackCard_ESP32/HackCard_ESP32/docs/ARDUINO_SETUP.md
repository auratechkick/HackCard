# HackCard Arduino Setup

## Board settings

Open **Arduino IDE → Tools** and use:

| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | **Enabled** (composite serial + HID; use web terminal if serial is quiet) |
| USB Mode | **USB-OTG (TinyUSB)** — required for HID keyboard |
| CPU Frequency | 240MHz |
| Flash Mode | QIO 80MHz |
| Flash Size | 4MB (32Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | Custom partition table |
| Upload Speed | 921600 |

## Custom partition table

This project includes `partitions.csv` in the sketch folder:

- ~3 MB application firmware (Huge APP)
- ~960 KB LittleFS for no-SD fallback

In Arduino IDE 2.x:

1. Open the `HackCard_ESP32` sketch folder
2. Set **Partition Scheme** to **Custom partition table**
3. Arduino should pick up `partitions.csv` automatically from the sketch directory

If your core version names the option differently, choose the closest **Huge APP (3MB No OTA / 1MB LittleFS)** scheme.

## Libraries

Install via **Tools → Manage Libraries**:

| Library | Purpose |
|---|---|
| Adafruit NeoPixel | 12-LED ring + WiFi RGB |
| ArduinoJson | Config load/save |
| Adafruit PN532 | NFC tag reading |
| Adafruit BusIO | Required by PN532 library |

## Personal info

Edit `config/user_config.h`:

- Your name, email, website
- AP SSID and password
- Buzzer and LED defaults

No SD card is required for Phase 0.

## Upload

1. Connect HackCard by USB-C
2. Select the correct COM port
3. Upload
4. Open Serial Monitor at **115200 baud**
5. Press RESET if needed

## Flash vs Full Mode

| Mode | When | Storage |
|---|---|---|
| Flash Mode | No SD inserted | LittleFS + compiled defaults |
| Full Mode | SD inserted | SD preferred, auto folder setup |

## Troubleshooting

### Wrong LED colors

In `config/board_config.h`, change:

```cpp
#define RGB_USE_GRB  1   // try 0 if red/green are swapped
```

### SD not detected

- Check SD is FAT32
- Run serial command: `sd status`
- Verify GPIO 10/11/12/13 wiring

### Web page not loading

- Connect to AP from `user_config.h`
- Open `http://192.168.4.1`
- Check serial log for `Status dashboard online`

### Upload fails

- Hold BOOT, press RESET, release RESET, release BOOT (download mode)
- Lower upload speed to 115200
