# Contributing

Thanks for improving HackCard.

## Before you open a PR

1. Test on **real HackCard ESP32-S3** hardware when changing firmware or UI.
2. Run `HackCard_ESP32/scripts/sync-web-ui.ps1` after editing `hackcard-web-ui/www/`.
3. Match existing code style in `src/` and comment style in examples (`release 1.0.0 example` headers).
4. Do not commit personal `user_config.h` secrets — use demo defaults.

## Documentation

- User docs: `docs/` at repo root  
- Firmware docs: `HackCard_ESP32/docs/`  
- Examples index: `HackCard_ESP32/library/HackCard/EXAMPLES.md`

## Ethics

Lab features must remain clearly scoped to authorized testing. See [docs/ethics-and-authorized-use.md](docs/ethics-and-authorized-use.md).
