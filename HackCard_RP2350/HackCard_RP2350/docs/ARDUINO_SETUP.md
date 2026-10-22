# Arduino setup

Same steps as [Getting started](../../docs/getting-started.md), kept next to the sketch.

| Tools | Value |
|-------|--------|
| Board | Generic RP2350 |
| Flash Size | 2MB (no FS) |
| USB Stack | Pico SDK |
| Port | The HackCard COM port |

Core: Earle Philhower RP2040/RP2350.  
Libraries: Adafruit PN532, Adafruit BusIO, Adafruit NeoPixel, plus this repo's **HackCard RP2350** library (`scripts/install-library.ps1`).

Upload failures: hold BOOT, tap RUN, release BOOT, then upload again.
