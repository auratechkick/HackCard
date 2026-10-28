# Personalize settings

Edit **`HackCard_ESP32/config/user_config.h`** before upload so Wi-Fi, profile, and security defaults match you — not factory demos.

After first boot, some values can also be changed from the Lab Console **Settings** page (stored in flash).

---

## Identity (Business profile)

Used for contact card, vCard export, BLE contact GATT, and example sketches:

| Define | Purpose |
|--------|---------|
| `USER_NAME`, `USER_TITLE`, `USER_COMPANY` | Profile header |
| `USER_EMAIL`, `USER_PHONE` | Contact fields |
| `USER_WEBSITE`, `USER_LINKEDIN`, `USER_GITHUB` | Links |
| `USER_BIO` | Short tagline |

Example:

<div class="code-meta" markdown="1">

**File:** `HackCard_ESP32/config/user_config.h`

</div>

```cpp
#define USER_NAME               "Alex Kim"
#define USER_TITLE              "Lab instructor"
#define USER_COMPANY            "Your organization"
#define USER_EMAIL              "lab@example.com"
#define USER_WEBSITE            "https://example.com"
#define USER_GITHUB             "https://github.com/auratechkick/HackCard"
```

---

## Wi-Fi access point

Clients (phone/laptop) join this network to reach the Lab Console:

| Define | Notes |
|--------|--------|
| `AP_SSID` | Default `HackCard-Setup` — pick something unique in a classroom |
| `AP_PASSWORD` | Min **8** characters — **change before sharing** |
| `AP_CHANNEL` | `0` = auto (2.4 GHz only) |

!!! warning "2.4 GHz only"
    ESP32 cannot create a 5 GHz AP. On some phones, disable “5 GHz only” or “smart connect” if the SSID does not appear.

---

## Security — Lab PIN and vault

| Define | Purpose |
|--------|---------|
| `LAB_MODE_PIN` | Unlocks **Lab mode** in the browser; confirms sensitive actions |
| `VAULT_PIN` | Extra PIN for vault-style features where enabled |

Defaults are for **local demo only**.

!!! danger "Change before resale or classroom handout"
    Set a strong `AP_PASSWORD` and a non-default `LAB_MODE_PIN`. Anyone who knows the defaults can unlock Wi-Fi and NFC lab tools on your AP.

---

## Comfort defaults

| Define | Purpose |
|--------|---------|
| `DEFAULT_RING_BRIGHTNESS` | 10–100 |
| `DEFAULT_BUZZER_VOLUME` | 10–100 |
| `BOOT_ANIMATION` | Startup ring animation |
| `BUZZER_ENABLED` | Master buzzer switch |
| `ACTIVITY_LOGGING` | Activity log when storage allows |
| `CAPTIVE_PORTAL_ENABLED` | Captive portal behavior for training demos |

---

## After editing

1. Save `user_config.h`.
2. Re-upload **`HackCard_ESP32.ino`** ([Flash firmware](first-program.md)).

To wipe saved profile/AP/PIN back to this file’s values later, use library example **`07_System/Factory_Reset`**, then flash main firmware again.

---

## Next step

→ [Flash firmware](first-program.md)
