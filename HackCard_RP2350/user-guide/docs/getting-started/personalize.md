# Personalize settings

Edit **`HackCard_RP2350/config/user_config.h`** before you share the board.

There is **no Wi-Fi AP** on RP2350 — identity fields are used for **`vcard`**, NDEF examples, and serial `status`.

---

## Identity (contact / vCard)

| Define | Purpose |
|--------|---------|
| `USER_NAME`, `USER_TITLE`, `USER_COMPANY` | Contact card |
| `USER_EMAIL`, `USER_PHONE` | Contact fields |
| `USER_WEBSITE`, `USER_LINKEDIN`, `USER_GITHUB` | Links |
| `USER_BIO` | Short line |

Example:

```cpp
#define USER_NAME               "Alex Kim"
#define USER_EMAIL              "lab@example.com"
#define USER_WEBSITE            "https://auratechkick.github.io/HackCard/rp2350/"
#define USER_GITHUB             "https://github.com/auratechkick/HackCard"
```

---

## Lab PIN

| Define | Default | Purpose |
|--------|---------|---------|
| `LAB_MODE_PIN` | **`1234`** | Reserved for lab-style confirmations in firmware/examples |

!!! danger "Change before handout"
    Default **`1234`** is public in this repo. Set your own PIN before classroom or client use.

There is **no** `AP_PASSWORD` on this board (no Wi-Fi).

---

## Comfort defaults

| Define | Purpose |
|--------|---------|
| `DEFAULT_RING_BRIGHTNESS` | 10–100 |
| `DEFAULT_BUZZER_VOLUME` | 10–100 |
| `BOOT_ANIMATION` | Startup animation |
| `BUZZER_ENABLED` | Master buzzer |
| `ACTIVITY_LOGGING` | Activity log when SD allows |
| `SD_AUTO_CREATE_FOLDERS` | Create lab folders on mount |

---

## After editing

Save and re-upload **`HackCard_RP2350.ino`**.

To clear **SD lab files** (not `user_config.h`), use example **`06_System/Factory_Reset`**, then flash main firmware again.

---

## Next step

→ [Flash firmware](first-program.md)
