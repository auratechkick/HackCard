# Copy RP2350-only library examples into ESP32 tree (adapt includes and paths).
$ErrorActionPreference = "Stop"
$rpRoot = "D:\HackCard_RP2350_Github\HackCard_RP2350\library\HackCard_RP2350\examples"
$espRoot = "$PSScriptRoot\..\library\HackCard\examples"

$folderMap = @{
  "01_Board"    = "01_Basics"
  "02_NFC"      = "02_NFC"
  "03_USB_HID"  = "05_USB_HID"
  "04_Storage"  = "05_Storage"
  "05_RealLife" = "06_RealLife"
  "06_System"   = "07_System"
}

$names = @(
  "01_Board/RGB_And_Button",
  "02_NFC/NFC_Dump_Pages",
  "03_USB_HID/HID_Copy_Contact",
  "03_USB_HID/HID_Desk_Remote",
  "03_USB_HID/HID_Ducky_Scripts",
  "03_USB_HID/HID_Email_Signature",
  "03_USB_HID/HID_Lock_And_Screenshot",
  "03_USB_HID/HID_Meeting_Notes",
  "03_USB_HID/HID_OS_Menu",
  "03_USB_HID/HID_Presentation_Clicker",
  "05_RealLife/Break_Note",
  "05_RealLife/Conference_Intro",
  "05_RealLife/Guest_Network_Card",
  "05_RealLife/Meeting_Link",
  "05_RealLife/NFC_Arrive_Leave",
  "05_RealLife/NFC_Business_Card",
  "05_RealLife/NFC_Desk_Presence",
  "05_RealLife/NFC_HID_Actions",
  "05_RealLife/NFC_HID_Lock",
  "05_RealLife/NFC_HID_Lock_Unlock",
  "05_RealLife/NFC_Tap_To_Type",
  "05_RealLife/Standup_Notes"
)

foreach ($rel in $names) {
  $parts = $rel -split "/"
  $srcCat = $parts[0]
  $sketch = $parts[1]
  $dstCat = $folderMap[$srcCat]
  if (-not $dstCat) { throw "No map for $srcCat" }

  $srcDir = Join-Path $rpRoot "$srcCat\$sketch"
  $dstDir = Join-Path $espRoot "$dstCat\$sketch"
  $srcIno = Join-Path $srcDir "$sketch.ino"
  if (-not (Test-Path $srcIno)) { throw "Missing $srcIno" }

  New-Item -ItemType Directory -Force -Path $dstDir | Out-Null
  $text = Get-Content $srcIno -Raw

  $text = $text -replace '#include\s*<HackCardRP2350\.h>', "#include <HackCard.h>`r`n#include <HackCardUsb.h>"
  $text = $text -replace 'HackCard RP2350 example 0\.1\.0', 'HackCard release 1.0.0 example (from RP2350)'
  $text = $text -replace 'HackCard RP2350 —', 'HackCard ESP32 —'
  $text = $text -replace 'Board: Generic RP2350[^\n]*\n', @'
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 *
'@
  $text = $text -replace 'Pico SDK', 'TinyUSB'

  if ($text -notmatch 'ETHICS:') {
    $text = $text -replace '(\*/\s*\r?\n)', "`$1`r`n * ETHICS: Use only on PCs, tags, and networks you own or may test.`r`n"
  }

  Set-Content -Path (Join-Path $dstDir "$sketch.ino") -Value $text -Encoding utf8
  Write-Host "Ported $dstCat/$sketch"
}

Write-Host "Done. $($names.Count) examples."
