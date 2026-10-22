# HackCard RP2350 examples (0.1.0)

Install the library first: `scripts/install-library.ps1`, then **File → Examples → HackCard RP2350**.

Board settings for every sketch: [ARDUINO_SETUP.md](../../HackCard_RP2350/docs/ARDUINO_SETUP.md).

This board has **no Wi-Fi and no BLE**. The ESP32 Wi-Fi and BLE examples are not in this tree.

| Example | What it does |
|---------|----------------|
| `00_Start_Here/Template_Custom_App` | Minimal ring, buzzer, button, optional NFC |
| `01_Board/RGB_And_Button` | Strip patterns and button gestures |
| `02_NFC/NFC_Read_UID` | Print tag UID |
| `02_NFC/NFC_Tag_Info` | UID, family, NDEF preview |
| `02_NFC/NFC_Write_URL` | NDEF URL on a tag you own |
| `02_NFC/NFC_Write_Text` | NDEF text on a tag you own |
| `02_NFC/NFC_Write_vCard` | NDEF vCard from constants you edit |
| `02_NFC/NFC_Dump_Pages` | Print Type 2 pages, and save them on the SD card |
| `02_NFC/NFC_Erase_Type2` | Clear the user area on a tag you own |
| `02_NFC/NFC_Kill_Type2` | Permanently lock a spare Type 2 tag after PIN and YES |
| `03_USB_HID/HID_Type_Hello` | Types a line into the focused window on your PC |
| `03_USB_HID/HID_Key_Press` | Sends Enter after a countdown |
| `03_USB_HID/HID_Mouse_Control` | Moves the pointer a short distance |
| `03_USB_HID/HID_Media_Keys` | Volume up and mute on this PC |
| `03_USB_HID/HID_OS_Menu` | Pick Windows, macOS, or Linux, then run shortcuts |
| `03_USB_HID/HID_Open_Website` | Open a URL with the shortcut for that OS |
| `03_USB_HID/HID_Lock_And_Screenshot` | Lock screen or take a screenshot |
| `03_USB_HID/HID_Presentation_Clicker` | Next, previous, and blank a slideshow |
| `03_USB_HID/HID_Email_Signature` | Type a signature into the focused window |
| `03_USB_HID/HID_Meeting_Notes` | Open notes and type a meeting template |
| `03_USB_HID/HID_Copy_Contact` | Type a contact and copy it to the clipboard |
| `03_USB_HID/HID_Desk_Remote` | Mute, volume, lock, and screenshot from one button |
| `03_USB_HID/HID_Ducky_Scripts` | Run Ducky Script samples, or a script from the SD card |
| `04_Storage/SD_Card_Info` | Mount SPI1 SD and list the root |
| `04_Storage/SD_Lab_Folders` | Create `/hackcard`, `/nfc`, `/payloads`, `/logs` |
| `05_RealLife/Desk_Status_Light` | Button cycles available / busy / away / focus |
| `05_RealLife/Conference_Badge_Tap` | NFC check-in, optional SD log |
| `05_RealLife/NFC_Business_Card` | Writes your vCard onto a tag |
| `05_RealLife/NFC_Tap_To_Type` | Badge tap types a check-in row |
| `05_RealLife/NFC_HID_Lock` | An enrolled tag locks this PC |
| `05_RealLife/NFC_HID_Lock_Unlock` | The same tag toggles lock and unlock |
| `05_RealLife/NFC_Desk_Presence` | Locks when your tag leaves, unlocks when it comes back |
| `05_RealLife/NFC_Arrive_Leave` | One tag unlocks and opens your page, another mutes and locks |
| `05_RealLife/NFC_HID_Actions` | Separate tags lock, screenshot, open a URL, type a signature, or mute |
| `05_RealLife/Conference_Intro` | Types your introduction into chat |
| `05_RealLife/Meeting_Link` | Opens your meeting URL for the selected OS |
| `05_RealLife/Guest_Network_Card` | Types your guest network into the focused field |
| `05_RealLife/Standup_Notes` | Opens notes and types a standup |
| `05_RealLife/Break_Note` | Types an away or back message into chat |
| `06_System/Diagnostics_Self_Test` | MCU, I2C, NFC, SD, button report |
| `06_System/Factory_Reset` | Clear SD lab files (tags, scripts, logs). Profile stays in user_config.h |

Production QC sketch (not an Arduino library example): `sketches/HackCard_Production_Test/`.
