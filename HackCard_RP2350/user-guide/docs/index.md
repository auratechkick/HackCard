<div class="hackcard-hero" markdown="1">

# HackCard RP2350

Credit-card sized **RP2350** lab board — NFC, USB HID, RGB, buzzer, and a **USB serial lab** at **115200** baud.

<span class="version-badge">Firmware v0.1.0</span>

[Start getting started :octicons-arrow-right-24:](getting-started/introduction.md){ .md-button .md-button--primary }

</div>

!!! tip "First session defaults"
    Serial **115200** → type **`help`**. Lab PIN in sketches/future screens: **`1234`** (change in `user_config.h`). Details: [Default PINs & serial](getting-started/default-credentials.md).

<div class="nfc-notice" markdown="1">

**No Wi-Fi / no web UI:** RP2350 has **no radio**. There is no `192.168.4.1` Lab Console — control the board from **Serial Monitor** and **USB HID** to your PC. For the browser dashboard, use **[HackCard ESP32](https://auratechkick.github.io/HackCard/esp32/)**.

</div>

---

## Recommended path (~30 minutes)

<ol class="backer-path" markdown="1">

<li>**[Introduction](getting-started/introduction.md)** — serial lab vs ESP32</li>
<li>**[What's on the board](getting-started/whats-included.md)**</li>
<li>**[Install tools & connect USB](getting-started/first-setup.md)**</li>
<li>**[Personalize settings](getting-started/personalize.md)**</li>
<li>**[Flash firmware](getting-started/first-program.md)**</li>
<li>**[Serial lab console](getting-started/serial-lab.md)** — `help`, NFC, HID, SD</li>
<li>**[Arduino examples](getting-started/arduino-examples.md)** — 40 sketches</li>
<li>**[Write your own sketch](getting-started/write-your-own-sketch.md)**</li>

</ol>

<div class="image-grid" markdown="1">

<figure class="hardware-image" markdown="1">
![HackCard front](assets/hardware/hackcard-front-render.png)
<figcaption>Front — RGB strip, user button</figcaption>
</figure>

<figure class="hardware-image" markdown="1">
![HackCard back](assets/hardware/hackcard-back-pcb.png)
<figcaption>Back — NFC coil, RP2350, USB-C, microSD</figcaption>
</figure>

</div>

---

## Verified specifications

| Spec | Value |
|------|-------|
| Board | HackCard RP2350 |
| MCU | RP2350 |
| Flash | 2 MB (W25Q16 — pick **2MB (no FS)** in Arduino) |
| Firmware | **0.1.0** |
| NFC | PN532 reader (I2C) |
| Interface | USB CDC serial + USB HID |
| Wi-Fi / BLE | **Not available** |

---

## Repository

**[github.com/auratechkick/HackCard](https://github.com/auratechkick/HackCard)** → `HackCard_RP2350/`
