# Copies library/HackCard_RP2350 into the Arduino sketchbook libraries folder.
# Safe to re-run. Does not touch a separate ESP32 library named HackCard.

$ErrorActionPreference = "Stop"

$source = Resolve-Path (Join-Path $PSScriptRoot "..\library\HackCard_RP2350")
$libraries = Join-Path $env:USERPROFILE "Documents\Arduino\libraries"
$dest = Join-Path $libraries "HackCard_RP2350"

if (-not (Test-Path $libraries)) {
  New-Item -ItemType Directory -Force -Path $libraries | Out-Null
}

if (Test-Path $dest) {
  Remove-Item $dest -Recurse -Force
}

Copy-Item $source $dest -Recurse
Write-Output "Installed HackCard RP2350 library to:"
Write-Output "  $dest"
Write-Output "Restart Arduino IDE, then File -> Examples -> HackCard RP2350."
