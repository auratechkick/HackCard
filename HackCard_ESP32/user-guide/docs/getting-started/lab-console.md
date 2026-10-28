# Open the Lab Console

The **Lab Console** is the web app served at **`http://192.168.4.1/`** when your phone or laptop is on the HackCard Wi-Fi access point.

UI source (for developers): `HackCard_ESP32/hackcard-web-ui/www/` — embedded in firmware after sync; **not** loaded from SD.

---

## Default login (first boot)

Unless you changed `user_config.h`:

| Item | Default |
|------|---------|
| Wi-Fi SSID | **`HackCard-Setup`** |
| Wi-Fi password | **`hackcard2026`** |
| Lab Console | **`http://192.168.4.1/`** |
| Lab PIN | **`1234`** |

Full list: [Default Wi-Fi, PINs, and URLs](default-credentials.md).

---

## Connect in four steps

1. **Power** HackCard via USB-C (stable power for Wi-Fi labs).
2. On phone or PC, open Wi-Fi settings and join the AP from the Serial boot banner (default SSID **`HackCard-Setup`**, password **`hackcard2026`**).
3. Open a browser to **`http://192.168.4.1/`** (use **http**, not https).
4. If the phone shows a captive portal, accept the network, then open the URL manually if the dashboard does not appear.

!!! note "Phone Wi-Fi tips"
    Use **2.4 GHz**. Disable “5 GHz only” or aggressive auto-switching if the SSID never appears.

---

## Business mode → Lab mode

| What you see first | Meaning |
|--------------------|---------|
| **Digital profile** | **Business mode** — contact page, QR, vCard download |
| Full sidebar / tabs | **Lab mode** — all labs unlocked |

To unlock Lab mode, enter **`LAB_MODE_PIN`** from `user_config.h` (or the PIN you set in Settings after a previous unlock).

On mobile, a PIN field may appear in the sidebar footer or in prompts before sensitive Wi-Fi/NFC actions.

---

## Lab mode — main areas

| Area | Typical use |
|------|-------------|
| **Dashboard** | Firmware version, Flash vs Full storage, AP IP, quick actions |
| **Wi-Fi Lab** | Scan, STA connect, AP settings, captive portal, training/monitor/karma/capture |
| **NFC Reader** | UID, tag info, dump/write, Type 2 tools, Classic labs |
| **Bluetooth LE** | Scan, advertise, contact card |
| **USB HID** | Type text, open URL, shortcuts — **requires USB to host PC** |
| **Files** | SD check, browse flash or SD, download/delete exports |
| **Settings** | AP password, Lab PIN, ring/buzzer |
| **Terminal** | Live log stream (similar family of messages as Serial @ 115200) |
| **Diagnostics** | Self-test style checks from the web |

Desktop: left **sidebar**. Narrow screens: bottom **tabs** and **More** for extra pages.

---

## Storage tier on the dashboard

| Tier | Meaning |
|------|---------|
| **Flash Mode** | No SD — labs run; SD export buttons disabled |
| **Full Mode** | SD mounted — **Export to SD** enabled for captures and dumps |

Insert a **FAT32** microSD before boot or use **Files → Check SD card** to mount.

---

## USB HID reminder

HID keystrokes go to the **USB host** (your PC), not the Wi-Fi client.

- Arduino **USB Mode** must be **USB-OTG (TinyUSB)**.
- Plug USB-C into the PC, focus a text field, then use HID page buttons.

---

## Safety

Wi-Fi monitor, karma, capture, and similar tools are for **authorized education** only. Change default PINs before demos.

Read: [Authorized use](../resources/authorized-use.md).

---

## Next step

→ [Next steps](next-steps.md) — examples, SD layout, troubleshooting
