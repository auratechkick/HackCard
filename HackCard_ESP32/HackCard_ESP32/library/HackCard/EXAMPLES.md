# HackCard Examples — feature index (firmware 1.0.0 · library 1.0.1 · 72 sketches)

Every major **Lab Console** capability has a matching **standalone Arduino example**.  
Upload **one sketch at a time**. Serial Monitor **115200** (USB CDC).

In-repo guide: [`docs/CUSTOM_DEVELOPMENT.md`](../../docs/CUSTOM_DEVELOPMENT.md)

---

## How to use

| Goal | Do this |
|------|---------|
| Find the right sketch | `00_Start_Here/Web_Feature_Map` or this table |
| Build your own app | Start from `00_Start_Here/Template_Custom_App` |
| Match a dashboard page | Use the **Web → Example** column below |
| Product-style demo | `06_RealLife/*` |

**Rules:** Edit **EDIT ME** at the top of each sketch. Lab sketches that touch RF/tags require the sketch **lab PIN** (default `1234` — change before sharing). Authorized use only.

---

## Complete web → example map

### Peripherals & UI (`01_Basics`)

| Lab Console / hardware | Example |
|------------------------|---------|
| RGB ring patterns | `01_Basics/Hello_RGB_Ring` |
| Ring demos 1–12 (`/apps`) | `01_Basics/LED_Ring_Demos` |
| Interactive ring API | `01_Basics/Ring_Control` |
| Wi-Fi symbol LED | `01_Basics/WiFi_Status_LED` |
| Buzzer tunes | `01_Basics/Buzzer_Tunes` |
| BOOT button gestures | `01_Basics/BOOT_Button` |
| RGB + button (RP2350 parity) | `01_Basics/RGB_And_Button` |

### NFC (`02_NFC`)

| Web `/nfc` | Example |
|------------|---------|
| Read UID | `02_NFC/NFC_Read_UID` |
| Tag info / NDEF preview | `02_NFC/NFC_Tag_Info` |
| Type 2 dump | `02_NFC/NFC_Dump_Type2` |
| Type 2 pages → serial + SD | `02_NFC/NFC_Dump_Pages` |
| Write URL | `02_NFC/NFC_Write_URL` |
| Write text | `02_NFC/NFC_Write_Text` |
| Write vCard | `02_NFC/NFC_Write_vCard` |
| Erase Type 2 | `02_NFC/NFC_Erase_Type2` |
| Clone Type 2 | `02_NFC/NFC_Clone_Type2` |
| Kill Type 2 | `02_NFC/NFC_Kill_Type2` |
| Classic dump | `02_NFC/NFC_Classic_Dump` |
| Classic clone | `02_NFC/NFC_Classic_Clone` |
| Classic keyscan | `02_NFC/NFC_Classic_Keyscan` |

### Wi-Fi (`03_WiFi`)

| Web `/wifi` | Example |
|-------------|---------|
| Scan networks | `03_WiFi/WiFi_Scanner` |
| STA connect | `03_WiFi/WiFi_Connect_STA` |
| Soft AP | `03_WiFi/WiFi_Access_Point` |
| Captive portal | `03_WiFi/WiFi_Captive_Portal` |
| Beacon lab | `03_WiFi/WiFi_Beacon_Lab` |
| Evil twin sim | `03_WiFi/WiFi_EvilTwin_Sim` |
| Training portal | `03_WiFi/WiFi_Training_Portal` |
| Monitor → probe sniff | `03_WiFi/WiFi_Probe_Sniff` |
| Monitor → deauth demo | `03_WiFi/WiFi_Deauth_Demo` |
| Attack → Karma | `03_WiFi/WiFi_Karma_Lab` |
| Capture → EAPOL | `03_WiFi/WiFi_EAPOL_Capture` |

### BLE (`04_BLE`)

| Web `/ble` | Example |
|------------|---------|
| Scan | `04_BLE/BLE_Scanner` |
| Advertise | `04_BLE/BLE_Advertise` |
| Contact card GATT | `04_BLE/BLE_Contact_Card_GATT` |

### USB HID (`05_USB_HID`)

| Web `/hid` | Example |
|------------|---------|
| Type text | `05_USB_HID/HID_Type_Hello` |
| Open URL | `05_USB_HID/HID_Open_Website` |
| Apps / lock | `05_USB_HID/HID_Apps_And_Lock` |
| Media keys | `05_USB_HID/HID_Media_Keys` |
| Mouse | `05_USB_HID/HID_Mouse_Control` |
| Key press | `05_USB_HID/HID_Key_Press` |
| OS menu shortcuts | `05_USB_HID/HID_OS_Menu` |
| Lock + screenshot | `05_USB_HID/HID_Lock_And_Screenshot` |
| Presentation clicker | `05_USB_HID/HID_Presentation_Clicker` |
| Email signature | `05_USB_HID/HID_Email_Signature` |
| Meeting notes template | `05_USB_HID/HID_Meeting_Notes` |
| Copy contact to clipboard | `05_USB_HID/HID_Copy_Contact` |
| One-button desk remote | `05_USB_HID/HID_Desk_Remote` |
| Ducky Script runner | `05_USB_HID/HID_Ducky_Scripts` |

### Storage (`05_Storage`)

| Web Settings → Storage | Example |
|------------------------|---------|
| SD card detect / list | `05_Storage/SD_Card_Info` |
| Full Mode folder layout | `05_Storage/SD_Lab_Folders` |

### System (`07_System`)

| Web page | Example |
|----------|---------|
| Diagnostics / home status | `07_System/Diagnostics_Self_Test` |
| Profile / vCard | `07_System/Profile_vCard_Export` |
| Modes / presets | `07_System/Mode_Status_Presets` |
| Activity log | `07_System/Activity_Log_Demo` |

### Real-life demos (`06_RealLife`)

| Scenario | Example |
|----------|---------|
| Conference badge NFC tap | `06_RealLife/Conference_Badge_Tap` |
| Wi-Fi guest portal | `06_RealLife/WiFi_Guest_Portal` |
| BLE business card | `06_RealLife/BLE_Business_Card` |
| Desk status light | `06_RealLife/Desk_Status_Light` |
| NFC business card write | `06_RealLife/NFC_Business_Card` |
| Tap tag → type check-in | `06_RealLife/NFC_Tap_To_Type` |
| Enrolled tag → lock PC | `06_RealLife/NFC_HID_Lock` |
| Tag toggles lock/unlock | `06_RealLife/NFC_HID_Lock_Unlock` |
| Desk presence lock/unlock | `06_RealLife/NFC_Desk_Presence` |
| Arrive / leave tag actions | `06_RealLife/NFC_Arrive_Leave` |
| Multi-tag HID actions | `06_RealLife/NFC_HID_Actions` |
| Conference intro (chat) | `06_RealLife/Conference_Intro` |
| Open meeting link | `06_RealLife/Meeting_Link` |
| Type guest Wi-Fi creds | `06_RealLife/Guest_Network_Card` |
| Standup notes template | `06_RealLife/Standup_Notes` |
| Break / back chat note | `06_RealLife/Break_Note` |

**RP2350 parity:** The rows above match the HackCard RP2350 library examples, adapted for ESP32-S3 (same NFC/HID flows; ESP32 also has Wi-Fi/BLE/web examples RP2350 does not).

---

## Suggested learning path

1. `Hello_RGB_Ring` → `Ring_Control` → `Buzzer_Tunes` → `BOOT_Button`
2. `NFC_Read_UID` → `NFC_Write_URL` → `NFC_Dump_Type2`
3. `WiFi_Access_Point` → `WiFi_Scanner` → `WiFi_Connect_STA`
4. `WiFi_Probe_Sniff` → `WiFi_Karma_Lab` (authorized lab only)
5. `BLE_Advertise` → `BLE_Contact_Card_GATT`
6. `HID_Type_Hello` → `HID_Open_Website` (TinyUSB board settings)
7. `Template_Custom_App` → your project

---

## Minimal custom app

```cpp
#include <HackCard.h>

void setup() {
  Serial.begin(115200);
  RgbRing.begin(60);
  Buzzer.begin(true);
  Buttons.begin();
  Nfc.begin();
}

void loop() {
  RgbRing.update();
  Buttons.update();
  // your logic
}
```

Copy `Template_Custom_App`, remove unused hardware, keep Serial prints for debugging.
