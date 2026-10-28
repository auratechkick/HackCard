# Authorized use

HackCard is a **security education and research platform**. Wi-Fi, NFC, and USB HID features can affect networks, tags, and computers around you.

---

## You may use HackCard when

- You **own** the hardware, tags, and computers involved, **or**
- You have **written permission** from the network/tag/system owner, **or**
- You run a **controlled lab or classroom** with clear rules and consent.

---

## You must not use HackCard to

- Access networks, accounts, or data **without authorization**
- Disrupt production Wi-Fi or impersonate real providers to harvest credentials outside a defined training exercise
- Clone or modify **payment cards**, **access badges**, or credentials you do not own
- Run USB HID payloads on machines **without the operator’s knowledge**

---

## Features that need extra care

| Feature | Risk | Mitigation |
|---------|------|------------|
| Wi-Fi monitor / deauth demo | RF disruption | Your AP / your lab only; follow local law |
| Karma / evil twin / training portal | Misleading SSIDs | Training only; disclose to participants |
| NFC clone / Classic tools | Tag copying | Tags you own; comply with local law |
| USB HID | Keystroke injection | Your PC only; informed users |

---

## Before sharing or selling a device

1. Change **`AP_PASSWORD`** and **`LAB_MODE_PIN`** in `user_config.h`.
2. Avoid training portal branding that mimics real companies.
3. Ship a short “authorized use only” note with the board.

---

## License note

Firmware and library software are under **MIT** (`HackCard_ESP32/LICENSE`). This page is guidance, not legal advice.
