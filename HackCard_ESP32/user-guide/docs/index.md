<div class="hackcard-hero" markdown="1">

# HackCard ESP32

Credit-card sized **ESP32-S3** lab hardware — NFC, Wi-Fi, BLE, USB HID, and a browser **Lab Console** at `http://192.168.4.1/`.

<span class="version-badge">Firmware v1.0.2</span>

[Start getting started :octicons-arrow-right-24:](getting-started/introduction.md){ .md-button .md-button--primary }

</div>

!!! tip "First login defaults"
    Wi-Fi **`HackCard-Setup`** / password **`hackcard2026`** → **`http://192.168.4.1/`** → Lab PIN **`1234`**. Full table: [Default Wi-Fi & PINs](getting-started/default-credentials.md).

<div class="nfc-notice" markdown="1">

**NFC:** HackCard has a **PN532 reader**. It reads and writes **external** tags. It does **not** emulate a payment or access card — your phone cannot tap the board as if it were a contactless card.

</div>

---

## Recommended path (about 30–45 minutes)

<ol class="backer-path" markdown="1">

<li>**[Introduction](getting-started/introduction.md)** — what the board is and is not</li>
<li>**[What's on the board](getting-started/whats-included.md)** — MCU, NFC, LEDs, SD, USB</li>
<li>**[Install tools & connect USB](getting-started/first-setup.md)** — Arduino IDE, ESP32 core, libraries</li>
<li>**[Personalize settings](getting-started/personalize.md)** — name, Wi-Fi AP, Lab PIN</li>
<li>**[Flash firmware](getting-started/first-program.md)** — upload and Serial Monitor check</li>
<li>**[Open the Lab Console](getting-started/lab-console.md)** — join Wi-Fi, Business vs Lab mode</li>
<li>**[Default Wi-Fi & PINs](getting-started/default-credentials.md)** — factory SSID, password, Lab PIN</li>
<li>**[Arduino examples](getting-started/arduino-examples.md)** — 73 sketches mapped to Lab Console</li>
<li>**[Write your own sketch](getting-started/write-your-own-sketch.md)** — template and custom apps</li>
<li>**[Next steps](getting-started/next-steps.md)** — storage, troubleshooting</li>

</ol>

<div class="image-grid" markdown="1">

<figure class="hardware-image" markdown="1">
![HackCard front](assets/hardware/hackcard-front-render.png)
<figcaption>Front — 12-LED RGB ring and Wi-Fi status LED</figcaption>
</figure>

<figure class="hardware-image" markdown="1">
![HackCard back PCB](assets/hardware/hackcard-back-pcb.png)
<figcaption>Back — NFC antenna, ESP32-S3, USB-C, microSD, RESET / BOOT</figcaption>
</figure>

</div>

---

## Verified specifications

| Spec | Value | Source |
|------|-------|--------|
| Board | HackCard ESP32-S3 | `HackCard_ESP32/config/board_config.h` |
| MCU | ESP32-S3FH4R2 | same |
| Flash / PSRAM | 4 MB / 2 MB OPI | same |
| Firmware | **1.0.2** | `FIRMWARE_VERSION` |
| NFC | PN532 (I2C reader) | same |
| Wi-Fi | **2.4 GHz only** | `user_config.h` |
| Web UI | Embedded Lab Console | `hackcard-web-ui/www/` → firmware |

---

## Repository

Firmware, library (**73** Arduino examples), and UI source:

**[github.com/auratechkick/HackCard](https://github.com/auratechkick/HackCard)** → folder `HackCard_ESP32/`

**RP2350 serial lab guide:** [auratechkick.github.io/HackCard/rp2350/](https://auratechkick.github.io/HackCard/rp2350/)
