# HackCard Wi-Fi Troubleshooting

## Why AP works on dev board but not HackCard PCB

Your HackCard uses pins that a generic ESP32-S3 dev board does **not**:

| Pin | Function | Risk |
|---|---|---|
| **GPIO 45** | 12x RGB ring | **ESP32-S3 strapping pin** — WS2812 activity can affect Wi-Fi/flash on custom PCBs |
| **GPIO 38** | WiFi symbol RGB | NeoPixel init after AP start can disturb RF |
| **GPIO 37** | Buzzer (LEDC PWM) | Known ESP32-S3 issue: PWM can break SoftAP visibility |
| **GPIO 8** | NFC SDA | Must not be held LOW at boot on ESP32-S3 |
| **GPIO 12** | SD CLK | SPI clock can add noise near PCB antenna |

Firmware now:
1. Starts **Wi-Fi first**
2. Keeps RGB/buzzer/SD **idle for 3–8 seconds**
3. Prints **`[wifi-diag]`** report on Serial Monitor
4. Detaches buzzer PWM when idle

## What to do now

1. Re-upload firmware
2. Open **Serial Monitor @ 115200**
3. **Scan Wi-Fi within 3 seconds of boot** (before LEDs start)
4. Look for network: **`HackCard-Setup`**
5. Copy the full **`[wifi-diag]`** block from serial and share it

## Serial lines to check

```
[wifi] AP IP: 192.168.4.1        ← software AP OK
[boot] Wi-Fi still OK after RGB init   ← GPIO45 did not kill AP
[boot] WARNING: Wi-Fi dropped after RGB init   ← GPIO45/38 likely culprit
```

## Web terminal commands

After connecting to AP (if visible):

```
wifi status
wifi diag
wifi restart
```

## Hardware checks (PCB)

1. **Antenna** — ESP32-S3 module antenna pad connected? Keep-out zone clear?
2. **GPIO 45** — WS2812 data line near antenna or with strong pull-up?
3. **USB power** — try powered USB hub or phone charger (not weak laptop port)
4. **2.4 GHz scan** — phone must show 2.4 GHz networks, not 5 GHz only

## If AP appears for 3 seconds then disappears

This confirms **software AP works** but **RGB/buzzer/SD kills RF**.

Temporary test: in `config/board_config.h` increase delay:

```cpp
#define WIFI_PERIPHERAL_DELAY_MS  60000   // 60 sec — more time to test Wi-Fi
```

If AP stays visible for 60 seconds, the PCB + GPIO conflict is confirmed.
