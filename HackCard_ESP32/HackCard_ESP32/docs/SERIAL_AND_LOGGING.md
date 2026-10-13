# Serial and logging (firmware 1.0.0)

## USB Serial console

| Setting | Value |
|---------|--------|
| Baud | **115200** |
| Init | `Serial.begin(115200)` in `setup()` (`HackCard_ESP32.ino`) |

On boot you should see tagged lines such as:

- `[boot] Wi-Fi first — RGB/SD deferred…`
- `[boot] Config source: flash|sd|nvs`
- `=== Connect to HackCard ===` (SSID, password, dashboard URL)

## HcSerial (mirrored output)

`HcSerial` (`src/core/HcSerial.cpp`) duplicates important text to:

1. **USB Serial** (Arduino `Serial`)
2. **SerialLogBuffer** — ring buffer read by the web UI **Terminal** page

Use **`HcSerial.println(...)`** for operator-facing boot/status messages.  
Use **`Log.info` / `Log.warn` / `Log.error`** for structured firmware logs.

## Logger

| API | Serial format | Persistent |
|-----|---------------|------------|
| `Log.info(tag, msg)` | `[ms][INFO][tag] msg` | No |
| `Log.warn(tag, msg)` | `[ms][WARN][tag] msg` | No |
| `Log.error(tag, msg)` | `[ms][ERROR][tag] msg` | No |
| `Log.activity(app, action, result)` | `[ms][ACT][app] action -> result` | Yes → `/logs/activity.csv` |

Activity CSV is written to **flash** when mounted; for `/wifi/` paths storage may also mirror to SD when the card is present.

Controlled by **`ACTIVITY_LOGGING`** in `config/user_config.h` (default on).

## Web terminal

The dashboard **Terminal** polls the serial log buffer populated by `HcSerial` and `Log`. It does not require `CLI_SERIAL_ENABLED`.

## CLI (optional)

`CLI_SERIAL_ENABLED` in `config/app_registry.h` defaults to **0** (web-first). When set to **1**, `CliEngine` reads commands from USB Serial at 115200 baud.

## Wi-Fi diagnostics

After deferred peripheral init, `WifiDiagPrintRuntimeReport()` may print RF/stack hints on Serial — useful when debugging AP visibility on 2.4 GHz.
