# Adds HackCard 1.0.0 standard header to library examples (idempotent).
$ErrorActionPreference = "Stop"
$examplesRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\library\HackCard\examples")).Path

$meta = @{
  "00_Start_Here\Web_Feature_Map\Web_Feature_Map.ino" = @{
    Title = "Web feature map (Serial cheat sheet)"
    Web = "Overview of all Lab Console pages"
    What = "Prints every dashboard area and matching example folder on USB Serial."
    Run4 = "Read the map, then open the folder you need in File -> Examples -> HackCard."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "00_Start_Here\Template_Custom_App\Template_Custom_App.ino" = @{
    Title = "Template - build your own app"
    Web = "Starting point instead of full HackCard_ESP32.ino firmware"
    What = "Minimal loop: ring, buzzer, BOOT, optional NFC tap and soft AP."
    Run4 = "Fork this sketch; delete features you do not need."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi (optional AP)"
  }
  "01_Basics\Hello_RGB_Ring\Hello_RGB_Ring.ino" = @{
    Title = "Hello RGB ring"
    Web = "Status ring patterns on Lab Console"
    What = "Cycles Idle, Scanning, Success, Error, Full/Flash mode patterns on the 12-LED ring."
    Run4 = "Watch the ring; pattern names print every 3 seconds."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "01_Basics\Ring_Control\Ring_Control.ino" = @{
    Title = "Ring control (interactive Serial)"
    Web = "Apps / status ring controls"
    What = "Type commands on Serial to set patterns, colors, demo IDs, pointer LED, brightness."
    Run4 = 'Type help on Serial for command list (p, c, d, @, b).'
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "01_Basics\LED_Ring_Demos\LED_Ring_Demos.ino" = @{
    Title = "LED ring demos 1-12"
    Web = "/apps ring demo"
    What = "Plays each built-in HalRgbRing demo pattern for 3 seconds."
    Run4 = "Let the loop cycle; demos repeat forever."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "01_Basics\WiFi_Status_LED\WiFi_Status_LED.ino" = @{
    Title = "Wi-Fi status LED"
    Web = "Wi-Fi icon LED states on dashboard"
    What = "Animates the single WS2812 near the Wi-Fi symbol through AP/STA/scan/error states."
    Run4 = "Watch GPIO 38 LED and Serial state names."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "01_Basics\Buzzer_Tunes\Buzzer_Tunes.ino" = @{
    Title = "Buzzer tunes"
    Web = "/apps buzzer settings"
    What = "Plays boot, success, error, and NFC-ready tunes on the PWM buzzer."
    Run4 = "Listen and match tunes to HalBuzzer API calls in code."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "01_Basics\BOOT_Button\BOOT_Button.ino" = @{
    Title = "BOOT button gestures"
    Web = "Physical menu / confirm on device"
    What = "Detects short, long, and double press on BOOT (GPIO 0) with ring + buzzer feedback."
    Run4 = "Press BOOT: short = pointer, long = success, double = error flash."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "02_NFC\NFC_Read_UID\NFC_Read_UID.ino" = @{
    Title = "NFC read UID"
    Web = "/nfc Read"
    What = "Polls PN532 and prints tag UID when a tag is present."
    Run4 = "Hold a tag on the coil; UID prints once per tap."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Tag_Info\NFC_Tag_Info.ino" = @{
    Title = "NFC tag info"
    Web = "/nfc Info"
    What = "Shows tag type and NDEF preview text when available."
    Run4 = "Tap tags; compare output with dashboard Info tab."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Dump_Type2\NFC_Dump_Type2.ino" = @{
    Title = "NFC Type 2 dump"
    Web = "/nfc Dump"
    What = "Reads NTAG/Ultralight pages and prints hex dump to Serial."
    Run4 = "Use your own writable Type 2 tag; dump prints once per tag."
    Lab = $true; TinyUsb = $false; Edit = $false
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Write_URL\NFC_Write_URL.ino" = @{
    Title = "NFC write URL"
    Web = "/nfc Write URL"
    What = "Writes an NDEF URL record to a blank Type 2 tag."
    Run4 = "Edit URL constant; place tag; confirm write on Serial."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Write_Text\NFC_Write_Text.ino" = @{
    Title = "NFC write text"
    Web = "/nfc Write text"
    What = "Writes plain NDEF text to Type 2."
    Run4 = "Edit message string; use a tag you own."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Write_vCard\NFC_Write_vCard.ino" = @{
    Title = "NFC write vCard"
    Web = "/nfc Write vCard / profile"
    What = "Encodes contact fields into an NDEF vCard on Type 2."
    Run4 = "Edit name/email/phone; write to your tag."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Erase_Type2\NFC_Erase_Type2.ino" = @{
    Title = "NFC erase Type 2"
    Web = "/nfc Erase"
    What = "Clears user data on a writable Type 2 tag (lab)."
    Run4 = "Confirm PIN if required; tag must be yours."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Clone_Type2\NFC_Clone_Type2.ino" = @{
    Title = "NFC clone Type 2"
    Web = "/nfc Clone"
    What = "Restores Type 2 content from a prior dump pattern (lab)."
    Run4 = "Follow Serial prompts; only on tags you own."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Kill_Type2\NFC_Kill_Type2.ino" = @{
    Title = "NFC kill Type 2"
    Web = "/nfc Kill"
    What = "Permanently locks a Type 2 tag (irreversible lab demo)."
    Run4 = "Use sacrificial tags only."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Classic_Dump\NFC_Classic_Dump.ino" = @{
    Title = "Mifare Classic dump"
    Web = "/nfc Classic dump"
    What = "Attempts Classic sector read/key flow; prints dump to Serial."
    Run4 = "Place Classic tag; authorized tags only."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Classic_Clone\NFC_Classic_Clone.ino" = @{
    Title = "Mifare Classic clone"
    Web = "/nfc Classic clone"
    What = "Lab clone flow from dump data (authorized use only)."
    Run4 = "Edit keys/dump source as marked in sketch."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "02_NFC\NFC_Classic_Keyscan\NFC_Classic_Keyscan.ino" = @{
    Title = "Mifare Classic keyscan"
    Web = "/nfc Classic keyscan"
    What = "Tries common keys against Classic sectors (training)."
    Run4 = "Own tags only; watch Serial for hits."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, Adafruit PN532, Adafruit BusIO"
  }
  "03_WiFi\WiFi_Scanner\WiFi_Scanner.ino" = @{
    Title = "Wi-Fi scanner"
    Web = "/wifi Scan"
    What = "Scans 2.4 GHz networks and prints SSID/RSSI/channel."
    Run4 = "Open Serial; scan runs periodically."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, WiFi"
  }
  "03_WiFi\WiFi_Connect_STA\WiFi_Connect_STA.ino" = @{
    Title = "Wi-Fi STA connect"
    Web = "/wifi Connect"
    What = "Joins your router as a Wi-Fi client (STA mode)."
    Run4 = "Edit SSID/password; upload; watch connect status."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi"
  }
  "03_WiFi\WiFi_Access_Point\WiFi_Access_Point.ino" = @{
    Title = "Wi-Fi soft AP"
    Web = "/wifi AP settings"
    What = "Starts HackCard as a 2.4 GHz access point (no web server in this sketch)."
    Run4 = "Join AP from phone; IP prints on Serial."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi"
  }
  "03_WiFi\WiFi_Captive_Portal\WiFi_Captive_Portal.ino" = @{
    Title = "Captive portal"
    Web = "/portal"
    What = "Soft AP plus minimal captive landing for training demos."
    Run4 = "Connect phone to AP; portal page opens when implemented in sketch."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, WebServer"
  }
  "03_WiFi\WiFi_Beacon_Lab\WiFi_Beacon_Lab.ino" = @{
    Title = "Wi-Fi beacon lab"
    Web = "/wifi Beacon lab"
    What = "Broadcasts custom beacon frames for RF lab (time-limited)."
    Run4 = "Edit SSID list; type lab PIN on Serial to start."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, esp_wifi"
  }
  "03_WiFi\WiFi_EvilTwin_Sim\WiFi_EvilTwin_Sim.ino" = @{
    Title = "Evil twin simulation"
    Web = "/wifi Evil twin"
    What = "Training AP that mimics a named network for awareness labs."
    Run4 = "Edit target SSID; authorized networks only."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi"
  }
  "03_WiFi\WiFi_Training_Portal\WiFi_Training_Portal.ino" = @{
    Title = "Training portal"
    Web = "/wifi/training"
    What = "Phishing-awareness style portal templates for classroom use."
    Run4 = "Follow Serial/AP instructions after upload."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, WebServer"
  }
  "03_WiFi\WiFi_Probe_Sniff\WiFi_Probe_Sniff.ino" = @{
    Title = "Probe request sniff"
    Web = "/wifi Monitor -> Probe sniff"
    What = "Promiscuous capture of probe requests and SSIDs for 60s."
    Run4 = "Type lab PIN on Serial; session runs once."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, esp_wifi"
  }
  "03_WiFi\WiFi_Deauth_Demo\WiFi_Deauth_Demo.ino" = @{
    Title = "Deauth demo (capped)"
    Web = "/wifi Monitor -> Deauth"
    What = "Sends a short capped deauth burst toward YOUR lab AP MAC only."
    Run4 = "Edit BSSID/channel; type PIN; own network only."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, esp_wifi"
  }
  "03_WiFi\WiFi_Karma_Lab\WiFi_Karma_Lab.ino" = @{
    Title = "Karma lab"
    Web = "/wifi Attack -> Karma"
    What = "Soft AP plus probe-response lure SSID (capped session)."
    Run4 = "Type PIN to start; type stop to end early."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, esp_wifi"
  }
  "03_WiFi\WiFi_EAPOL_Capture\WiFi_EAPOL_Capture.ino" = @{
    Title = "EAPOL capture counter"
    Web = "/wifi Capture"
    What = "Counts EAPOL frames on a fixed channel; optional SD log line."
    Run4 = "Type PIN; wait for session end; check Serial count."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, esp_wifi, SD optional"
  }
  "04_BLE\BLE_Scanner\BLE_Scanner.ino" = @{
    Title = "BLE scanner"
    Web = "/ble Scan"
    What = "Scans nearby BLE devices and prints address/RSSI/name."
    Run4 = "Open Serial; scan results stream periodically."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, BLE"
  }
  "04_BLE\BLE_Advertise\BLE_Advertise.ino" = @{
    Title = "BLE advertise"
    Web = "/ble Advertise"
    What = "Advertises a custom BLE name from the HackCard."
    Run4 = "Use nRF Connect on phone to see the device."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, BLE"
  }
  "04_BLE\BLE_Contact_Card_GATT\BLE_Contact_Card_GATT.ino" = @{
    Title = "BLE contact card GATT"
    Web = "/ble Contact card"
    What = "Exposes name/URL over GATT like the dashboard contact card."
    Run4 = "Pair/read from a phone BLE app."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, BLE"
  }
  "05_USB_HID\HID_Type_Hello\HID_Type_Hello.ino" = @{
    Title = "USB HID type hello"
    Web = "/hid Type text"
    What = "Types a short message into the focused PC window via USB keyboard."
    Run4 = "Focus Notepad before countdown ends."
    Lab = $true; TinyUsb = $true; Edit = $false
    Deps = "USB, USBHIDKeyboard (ESP32 TinyUSB)"
  }
  "05_USB_HID\HID_Open_Website\HID_Open_Website.ino" = @{
    Title = "USB HID open website"
    Web = "/hid Open URL"
    What = "Opens browser and navigates to a URL using keyboard shortcuts."
    Run4 = "Edit URL; focus desktop; wait for countdown."
    Lab = $true; TinyUsb = $true; Edit = $true
    Deps = "USB, USBHIDKeyboard"
  }
  "05_USB_HID\HID_Apps_And_Lock\HID_Apps_And_Lock.ino" = @{
    Title = "USB HID apps and lock"
    Web = "/hid Apps / Lock"
    What = "Demonstrates Win/Linux shortcut to lock or open Run dialog."
    Run4 = "Select OS in EDIT ME; test on your own PC."
    Lab = $true; TinyUsb = $true; Edit = $true
    Deps = "USB, USBHIDKeyboard"
  }
  "05_USB_HID\HID_Media_Keys\HID_Media_Keys.ino" = @{
    Title = "USB HID media keys"
    Web = "/hid Media"
    What = "Sends volume/play media key reports."
    Run4 = "Focus a media app on host PC."
    Lab = $true; TinyUsb = $true; Edit = $false
    Deps = "USB, USBHIDKeyboard"
  }
  "05_USB_HID\HID_Mouse_Control\HID_Mouse_Control.ino" = @{
    Title = "USB HID mouse"
    Web = "/hid Mouse"
    What = "Moves mouse cursor in a small pattern."
    Run4 = "Watch cursor on host after countdown."
    Lab = $true; TinyUsb = $true; Edit = $false
    Deps = "USB, USBHIDMouse"
  }
  "05_USB_HID\HID_Key_Press\HID_Key_Press.ino" = @{
    Title = "USB HID key press"
    Web = "/hid Key press"
    What = "Sends individual key down/up codes."
    Run4 = "Edit key constant; focus text field on PC."
    Lab = $true; TinyUsb = $true; Edit = $true
    Deps = "USB, USBHIDKeyboard"
  }
  "05_Storage\SD_Card_Info\SD_Card_Info.ino" = @{
    Title = "SD card info"
    Web = "Settings -> Storage (Full Mode)"
    What = "Mounts microSD over SPI and lists root directory."
    Run4 = "Insert FAT32 card; read size and file list on Serial."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, SD, SPI"
  }
  "05_Storage\SD_Lab_Folders\SD_Lab_Folders.ino" = @{
    Title = "SD lab folders"
    Web = "Settings -> Storage export paths"
    What = "Creates /nfc, /wifi/exports, /payloads, etc. on SD (matches firmware)."
    Run4 = "Run once per new card before Full Mode exports."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard, SD, SPI"
  }
  "06_RealLife\Conference_Badge_Tap\Conference_Badge_Tap.ino" = @{
    Title = "Conference badge tap"
    Web = "Combined NFC + LED demo"
    What = "Success animation when a badge tag is read."
    Run4 = "Tap tag; ring and buzzer confirm."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard"
  }
  "06_RealLife\WiFi_Guest_Portal\WiFi_Guest_Portal.ino" = @{
    Title = "Wi-Fi guest portal demo"
    Web = "/portal + AP"
    What = "End-to-end guest Wi-Fi + landing page style demo."
    Run4 = "Join AP from phone; follow Serial URL."
    Lab = $true; TinyUsb = $false; Edit = $true
    Deps = "HackCard, WiFi, WebServer"
  }
  "06_RealLife\BLE_Business_Card\BLE_Business_Card.ino" = @{
    Title = "BLE business card"
    Web = "/ble Contact"
    What = "Advertises contact info for conference networking."
    Run4 = "Scan with phone; edit name/URL in EDIT ME."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard, BLE"
  }
  "06_RealLife\Desk_Status_Light\Desk_Status_Light.ino" = @{
    Title = "Desk status light"
    Web = "Ring as status indicator"
    What = "Uses ring colors for busy/available/focus states via Serial commands."
    Run4 = "Type 1/2/3 on Serial to switch states."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "07_System\Diagnostics_Self_Test\Diagnostics_Self_Test.ino" = @{
    Title = "Diagnostics self-test"
    Web = "/diag"
    What = "Checks ring, buzzer, button, NFC, SD presence and prints report."
    Run4 = "Read PASS/FAIL lines on Serial."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "07_System\Profile_vCard_Export\Profile_vCard_Export.ino" = @{
    Title = "Profile vCard export"
    Web = "/profile /setup"
    What = "Builds vCard text from EDIT ME contact fields and prints to Serial."
    Run4 = "Copy vCard from Serial to contacts app."
    Lab = $false; TinyUsb = $false; Edit = $true
    Deps = "HackCard"
  }
  "07_System\Mode_Status_Presets\Mode_Status_Presets.ino" = @{
    Title = "Mode status presets"
    Web = "/modes"
    What = "Cycles ring patterns used for Flash vs Full storage tier indicators."
    Run4 = "Watch ring when switching modes on Serial."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
  "07_System\Activity_Log_Demo\Activity_Log_Demo.ino" = @{
    Title = "Activity log demo"
    Web = "/logs"
    What = "Appends CSV-style activity lines to Serial (full firmware writes flash/SD)."
    Run4 = "Trigger events; lines print like dashboard activity log."
    Lab = $false; TinyUsb = $false; Edit = $false
    Deps = "HackCard"
  }
}

function Build-Header($m) {
  $lines = @(
    "/*",
    " * HackCard release 1.0.0 example",
    " * -------------------------------------------------------------------------",
    " * FEATURE: $($m.Title)",
    " * FULL FIRMWARE: $($m.Web)",
    " *",
    " * WHAT IT DOES:",
    " * $($m.What)",
    " *",
    " * BEFORE UPLOAD (Arduino IDE -> Tools):",
    " *   Board ................ ESP32S3 Dev Module",
    " *   USB CDC On Boot ...... Enabled"
  )
  if ($m.TinyUsb) {
    $lines += " *   USB Mode ............. USB-OTG (TinyUSB)"
  }
  $lines += @(
    " *",
    " * HOW TO RUN:",
    " *   1. File -> Examples -> HackCard -> (this sketch's folder)",
    " *   2. $(if ($m.Edit) { 'Edit constants marked EDIT ME at top of file.' } else { 'No edits required for first test.' })",
    " *   3. Upload and open Serial Monitor @ 115200 baud",
    " *   4. $($m.Run4)",
    " *",
    " * LIBRARIES: $($m.Deps)",
    " * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md"
  )
  if ($m.Lab) {
    $lines += " * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test."
  }
  $lines += " */"
  return ($lines -join "`n")
}

function Strip-OldHeader($content) {
  if ($content -match '(?s)^/\*.*?\*/\s*') {
    return $content -replace '(?s)^/\*.*?\*/\s*', ''
  }
  return $content
}

$updated = 0
Get-ChildItem -Path $examplesRoot -Recurse -Filter "*.ino" | ForEach-Object {
  $rel = $_.FullName.Substring($examplesRoot.Length).TrimStart('\', '/')
  if (-not $meta.ContainsKey($rel)) {
    Write-Warning "No metadata for $rel"
    return
  }
  $body = [IO.File]::ReadAllText($_.FullName)
  if ($body -match 'HackCard release 1\.0\.0 example') {
    return
  }
  $header = Build-Header $meta[$rel]
  $rest = Strip-OldHeader $body
  [IO.File]::WriteAllText($_.FullName, ($header + "`n`n" + $rest.TrimStart()))
  $updated++
  Write-Host "Updated $rel"
}

Write-Host "Done. Updated $updated sketch(es)."
