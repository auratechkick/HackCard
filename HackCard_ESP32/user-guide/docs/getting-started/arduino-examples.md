# Arduino example library

The **HackCard** library includes **73** standalone sketches. Each one teaches a single Lab Console feature (or a real-life demo) without loading the full web firmware.

Path in the repo:

`HackCard_ESP32/HackCard_ESP32/library/HackCard/examples/`

---

## How to run an example

1. Install the library: copy the `HackCard` folder to `Documents/Arduino/libraries/HackCard`, or open examples from the cloned repo.
2. Arduino IDE → **File → Examples → HackCard →** pick a folder and sketch.
3. Edit the **`// EDIT ME`** block at the top (PINs, URLs, SSIDs, text to type).
4. **Tools** → same board settings as full firmware ([First setup](first-setup.md)).
5. Upload **one sketch at a time** (each `.ino` is a complete program).
6. Open **Serial Monitor @ 115200** and follow on-screen prompts.

!!! tip "Map the whole product"
    Start with **`00_Start_Here/Web_Feature_Map`** — it prints which example matches each Lab Console area.

---

## Example folders

| Folder | What you learn |
|--------|----------------|
| **`00_Start_Here`** | Feature map, custom app template |
| **`01_Basics`** | RGB ring, buzzer, BOOT button, Wi-Fi LED — no Wi-Fi lab required |
| **`02_NFC`** | Read UID, dump/write tags, Type 2 erase/clone/kill, MIFARE Classic labs |
| **`03_WiFi`** | Scan, AP, STA, captive portal, beacon, training portal, monitor, karma, EAPOL capture |
| **`04_BLE`** | Scan, advertise, GATT contact card |
| **`05_USB_HID`** | Keyboard text, URLs, OS shortcuts, mouse, media keys, Ducky Script, desk workflows |
| **`05_Storage`** | SD info, export folder layout |
| **`06_RealLife`** | Badge tap, desk status, NFC lock/unlock, guest Wi-Fi notes, meeting helpers |
| **`07_System`** | Profile/vCard, modes, diagnostics, activity log, **Factory_Reset** |

---

## Match a Lab Console page to an example

| Lab Console area | Start with these examples |
|------------------|---------------------------|
| Dashboard / status | `01_Basics/Hello_RGB_Ring`, `Web_Feature_Map` |
| NFC Reader | `02_NFC/NFC_Read_UID`, `NFC_Write_URL`, `NFC_Dump_Type2` |
| Wi-Fi Lab | `03_WiFi/WiFi_Scanner`, `WiFi_Access_Point`, `WiFi_Captive_Portal` |
| BLE | `04_BLE/BLE_Scanner`, `BLE_Contact_Card_GATT` |
| USB HID | `05_USB_HID/HID_Type_Text`, `HID_Open_Website` |
| System / reset | `07_System/Factory_Reset`, `Diagnostics_Self_Test` |

The full web → sketch table lives in the repo:  
[EXAMPLES.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/library/HackCard/EXAMPLES.md)

---

## PINs and ethics in examples

- Sketches that erase, clone, or run Wi-Fi offensive demos may ask for a **lab PIN** — default **`1234`** unless you changed `LAB_MODE_PIN` in firmware.
- Use only on **tags, networks, and PCs you own** or have **written permission** to test.  
  See [Authorized use](../resources/authorized-use.md).

---

## Next

→ [Write your own sketch](write-your-own-sketch.md)
