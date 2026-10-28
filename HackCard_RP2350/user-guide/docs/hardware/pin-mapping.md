# Pin mapping (RP2350)

Authoritative source: [`HackCard_RP2350/config/board_config.h`](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/HackCard_RP2350/config/board_config.h) and [`HackCard_Pins.h`](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/library/HackCard_RP2350/src/HackCard_Pins.h).

Verified against the production-test sketch (`sketches/HackCard_Production_Test/`).

Board: **HackCard RP2350** · **2 MB** flash (W25Q16 — Arduino **2MB (no FS)**).

---

## GPIO and buses

| Function | GPIO | Bus / notes |
|----------|------|-------------|
| **User button** | **2** | Active low, internal pull-up |
| **Buzzer** | **9** | `tone()` / `noTone()` |
| **PN532 SDA** | **6** | **I2C1** (`Wire1`) |
| **PN532 SCL** | **7** | **I2C1** (`Wire1`) |
| **PN532 IRQ** | **11** | Present; many reads poll instead |
| **PN532 address** | **0x24** | Same as ESP32 board |
| **RGB strip** | **24** | 12× WS2812, GRB — pixel 0 often used as status |
| **SD CS** | **25** | **SPI1** |
| **SD CLK** | **26** | **SPI1** |
| **SD MOSI** | **27** | **SPI1** |
| **SD MISO** | **28** | **SPI1** |
| **USB-C** | — | Power + USB CDC serial + USB HID |

Use **`Wire1`** and **`SPI1`** in sketches — default `Wire` / `SPI` do not reach the HackCard connectors.

---

## Not available on this PCB

| Feature | Notes |
|---------|--------|
| **Wi-Fi / BLE** | Not on RP2350 — use **HackCard ESP32-S3** for radio labs |
| **Lab Console web UI** | Serial **115200** only |
| **BOOT / RUN as GPIO** | Hardware boot/reset — not tested as lab inputs |

---

## Storage and NFC

- microSD optional, **FAT32**.
- PN532 is **reader/writer** — not phone-style card emulation.

---

## Related docs

- [What's on the board](../getting-started/whats-included.md)
- In-repo reference: [hardware.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/docs/hardware.md)
