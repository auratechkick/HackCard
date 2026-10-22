# Serial console

115200 baud, newline (NL or CR+LF).

| Command | Action |
|---------|--------|
| `help` | List commands |
| `status` | MCU, NFC, SD, owner |
| `rgb idle\|scan\|ok\|err\|off` | Ring pattern |
| `beep` | Chime |
| `nfc` | One tag read |
| `watch` | Toggle continuous reads |
| `url https://example.com` | NDEF URL onto a tag you own |
| `text hello` | NDEF text onto a tag you own |
| `vcard` | Contact from `user_config.h` |
| `sd` | Mount, create lab folders, list root |
| `hid hello` | After 3 seconds, type that line on this PC |
| `os` | Show the selected OS |
| `os win` / `os mac` / `os linux` | Choose shortcut style |
| `site https://example.com` | Open that page on this PC |
| `notes` | Open Notepad, TextEdit, or gedit |
| `lock` | Lock this PC |
| `shot` | Screenshot on this PC |
| `log` | Last lines (also `/logs/activity.log` if SD is mounted) |

`hid` only affects the computer the board is plugged into. Focus your own window during the countdown.
