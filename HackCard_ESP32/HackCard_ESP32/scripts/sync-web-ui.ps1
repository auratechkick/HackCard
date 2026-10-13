# Sync hackcard-web-ui/www into firmware data/ for LittleFS upload (no SD required)
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$src = Join-Path $root "hackcard-web-ui\www"
$dst = Join-Path $PSScriptRoot "..\data\www"
if (-not (Test-Path $src)) {
  Write-Error "Source not found: $src"
  exit 1
}
New-Item -ItemType Directory -Force -Path "$dst\assets\css", "$dst\assets\js" | Out-Null
Copy-Item "$src\index.html" "$dst\" -Force
Copy-Item "$src\profile.html" "$dst\" -Force
Copy-Item "$src\assets\css\*" "$dst\assets\css\" -Force
Copy-Item "$src\assets\js\*" "$dst\assets\js\" -Force
& "$PSScriptRoot\generate-embedded-ui.ps1"
Write-Host "Synced to $dst - UI embedded in firmware; optional: Tools -> ESP32 Sketch Data Upload"
