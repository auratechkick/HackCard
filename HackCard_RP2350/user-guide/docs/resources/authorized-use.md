# Authorized use

HackCard RP2350 can read/write **NFC tags** and send **USB HID** keystrokes to a connected PC.

---

## You may use HackCard when

- You **own** the hardware, tags, and PC, **or**
- You have **written permission** from the owner, **or**
- You run a **controlled lab or classroom** with clear rules.

---

## You must not use HackCard to

- Clone or modify **access badges** or credentials you do not own
- Run HID or Ducky payloads on machines **without the operator’s knowledge**
- Use NFC erase/kill on tags that are not yours or not explicitly designated for lab destruction

---

## Before sharing a device

1. Change **`LAB_MODE_PIN`** in `user_config.h`.
2. Remove enrolled tag UIDs from lock/unlock sketches or re-flash examples with your own policy.
3. Include an “authorized use only” note.

---

## License note

Software is **MIT** (`HackCard_RP2350/LICENSE`). This page is guidance, not legal advice.
