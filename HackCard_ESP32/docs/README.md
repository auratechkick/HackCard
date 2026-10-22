# HackCard documentation

Welcome. Pick a path below — from first power-on to firmware and API development.

## Choose your path

| You are… | Start here | Time |
|----------|------------|------|
| **New owner — first power-on** | [Getting started](getting-started.md) | ~15 min |
| **Using the web dashboard only** | [Lab Console user guide](lab-console-user-guide.md) | ~20 min read |
| **Flashing / updating firmware** | [Arduino setup](../HackCard_ESP32/docs/ARDUINO_SETUP.md) | ~30 min first time |
| **Writing your own sketches** | [Custom development](../HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md) + [Examples index](../HackCard_ESP32/library/HackCard/EXAMPLES.md) | Ongoing |
| **Integrating with the REST API** | [API overview](api-overview.md) | Reference |

## Documentation map

### Users (beginner → intermediate)

| Topic | Document |
|-------|----------|
| Unbox, flash, connect, first login | [getting-started.md](getting-started.md) |
| Lab Console pages, PIN, mobile UI | [lab-console-user-guide.md](lab-console-user-guide.md) |
| Backer tips (profile, button, AP) | [BACKER_GUIDE.md](../HackCard_ESP32/docs/BACKER_GUIDE.md) |
| Flash vs SD storage | [STORAGE.md](../HackCard_ESP32/docs/STORAGE.md) |
| Wi-Fi not visible / 5 GHz phones | [WIFI_TROUBLESHOOTING.md](../HackCard_ESP32/docs/WIFI_TROUBLESHOOTING.md) |
| Pin map & peripherals | [hardware.md](hardware.md) |
| Serial Monitor & web Terminal | [SERIAL_AND_LOGGING.md](../HackCard_ESP32/docs/SERIAL_AND_LOGGING.md) |
| Authorized use & lab safety | [ethics-and-authorized-use.md](ethics-and-authorized-use.md) |

### Developers (intermediate → expert)

| Topic | Document |
|-------|----------|
| Firmware layout & modes | [architecture.md](architecture.md) |
| Edit web UI → ship in firmware | [hackcard-web-ui/README.md](../hackcard-web-ui/README.md) |
| REST API used by the dashboard | [api-overview.md](api-overview.md) |
| 73 Arduino examples (incl. factory reset and RP2350-parity HID/NFC) | [EXAMPLES.md](../HackCard_ESP32/library/HackCard/EXAMPLES.md) |
| Release / test checklist | [RELEASE.md](../HackCard_ESP32/RELEASE.md) |
| Changelog | [CHANGELOG.md](../HackCard_ESP32/CHANGELOG.md) |

## Version

Documentation matches firmware **1.0.2** (`HackCard_ESP32/config/board_config.h`), including the Lab Console **Files** page. Library **1.0.2** adds factory reset — see [CHANGELOG](../HackCard_ESP32/CHANGELOG.md).

**HackCard RP2350** (serial lab, no Wi‑Fi): [../../HackCard_RP2350/docs/README.md](../../HackCard_RP2350/docs/README.md)
