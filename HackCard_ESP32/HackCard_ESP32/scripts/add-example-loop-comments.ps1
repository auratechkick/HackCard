# Adds short setup/loop comments to release 1.0.0 examples (idempotent).
$examplesRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\library\HackCard\examples")).Path
$count = 0

Get-ChildItem -Path $examplesRoot -Recurse -Filter "*.ino" | ForEach-Object {
  $text = [IO.File]::ReadAllText($_.FullName)
  if ($text -notmatch 'HackCard release 1\.0\.0 example') { return }

  $orig = $text
  if ($text -notmatch '// setup\(\) -') {
    $text = $text -replace '(?m)^void setup\(\) \{\s*$', "// setup() - board init, Serial, peripherals`r`nvoid setup() {"
  }
  if ($text -notmatch '// loop\(\) -') {
    $text = $text -replace '(?m)^void loop\(\) \{\s*$', "// loop() - keep calling Hal*.update() here`r`nvoid loop() {"
  }
  # Collapse duplicate setup/loop comment lines from re-runs
  $text = $text -replace '(?m)^// setup\(\) -[^\r\n]*\r?\n(?=// setup\(\) -)', ''
  $text = $text -replace '(?m)^// loop\(\) -[^\r\n]*\r?\n(?=// loop\(\) -)', ''
  if ($text -ne $orig) {
    [IO.File]::WriteAllText($_.FullName, $text)
    $count++
  }
}
Write-Host "Added section comments to $count file(s)."
