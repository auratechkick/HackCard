# Arduino example library

**40** sketches under **File → Examples → HackCard RP2350** (after `scripts/install-library.ps1`).

Path: `HackCard_RP2350/library/HackCard_RP2350/examples/`

There are **no Wi-Fi or BLE** examples on this board.

---

## How to run an example

1. Run **`scripts/install-library.ps1`** once.
2. **Tools** → **Generic RP2350**, **2MB (no FS)**, **USB Stack: Pico SDK**.
3. **File → Examples → HackCard RP2350** → pick one sketch.
4. Edit **`// EDIT ME`** at the top (PINs, URLs, enrolled tag UIDs).
5. Upload **one sketch at a time**.
6. Serial Monitor **115200**.

---

## Example folders

| Folder | Topics |
|--------|--------|
| **`00_Start_Here`** | `Template_Custom_App` |
| **`01_Board`** | RGB strip + button gestures |
| **`02_NFC`** | Read, write URL/text/vCard, dump pages, erase, kill Type 2 |
| **`03_USB_HID`** | Type text, mouse, media, OS menus, Ducky Script, desk remote |
| **`04_Storage`** | SD info, lab folder creation |
| **`05_RealLife`** | Desk status, badge tap, NFC→HID lock/unlock, meeting/guest notes |
| **`06_System`** | Diagnostics, **Factory_Reset** |

---

## Map serial lab features to examples

| Serial / habit | Try these examples |
|----------------|-------------------|
| `nfc` / tag read | `02_NFC/NFC_Read_UID`, `NFC_Tag_Info` |
| Write tag | `NFC_Write_URL`, `NFC_Write_vCard` |
| `hid` on PC | `03_USB_HID/HID_Type_Hello`, `HID_Open_Website` |
| Ducky | `03_USB_HID/HID_Ducky_Scripts` |
| SD folders | `04_Storage/SD_Lab_Folders` |
| NFC lock PC | `05_RealLife/NFC_HID_Lock_Unlock` |

Full table: [EXAMPLES.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/library/HackCard_RP2350/EXAMPLES.md)

---

## PINs and ethics

- Destructive NFC labs (erase/kill) and lock/unlock sketches require tags and PCs **you own** or have permission to use.
- Default lab PIN **`1234`** if the sketch asks for one — match `LAB_MODE_PIN` in `user_config.h`.

→ [Authorized use](../resources/authorized-use.md)

---

## Next

→ [Write your own sketch](write-your-own-sketch.md)
