# Deprecated in firmware 1.0.0 — dashboard UI is not read from SD /www/.
Write-Host "deploy-to-sd.ps1 is deprecated for the Lab Console." -ForegroundColor Yellow
Write-Host "Use HackCard_ESP32\scripts\sync-web-ui.ps1 and upload firmware instead."
Write-Host "SD is for lab exports (/wifi, /nfc, /payloads) — see HackCard_ESP32\docs\STORAGE.md"
exit 1
