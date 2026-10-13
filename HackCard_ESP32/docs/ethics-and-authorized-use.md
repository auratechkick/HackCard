# Ethics and authorized use

HackCard is a **security education and research platform**. Some features interact with Wi-Fi radio, NFC tags, and USB input on a connected computer.

## You may use HackCard when

- You own the hardware, tags, and computers involved, **or**
- You have **written permission** from the owner of the network, tags, or systems under test, **or**
- You are in a **controlled classroom / lab** with explicit rules and consent.

## You must not use HackCard to

- Access networks, accounts, or data without authorization  
- Disrupt production Wi-Fi or impersonate real service providers to harvest credentials without a defined training exercise  
- Clone or modify payment cards, access badges, or credentials you do not own  
- Deploy HID payloads on machines without the operator’s knowledge  

## Lab features that deserve extra care

| Feature | Risk | Mitigation |
|---------|------|------------|
| Wi-Fi monitor / deauth demo | RF disruption | Own AP only; capped bursts in firmware/examples |
| Karma / evil twin / training portal | Misleading SSIDs | Training only; clear disclosure to participants |
| NFC clone / Classic tools | Tag copying | Tags you own; local law compliance |
| USB HID | Keystroke injection | Your PC only; informed users |

## Before sharing or selling a device

1. Change **`AP_PASSWORD`** and **`LAB_MODE_PIN`** in `user_config.h`.  
2. Remove or rewrite training portal branding that mimics real brands.  
3. Include a short “authorized use” note with the hardware.

## License

Software is under [MIT](../HackCard_ESP32/LICENSE). Hardware may have separate terms from the vendor.  
This document is guidance, not legal advice.
