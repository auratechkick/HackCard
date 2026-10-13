/*
 * HackCard ESP32 — Release firmware (see FIRMWARE_VERSION in config/board_config.h)
 *
 * Personalize AP, Lab PIN, and profile in: config/user_config.h
 * Feature flags: config/app_registry.h
 */

#include "config/board_config.h"
#include "config/app_registry.h"
#include "config/user_config.h"
#include "src/core/AppContext.h"
#include "src/core/CliEngine.h"
#include "src/core/ConfigStore.h"
#include "src/core/HcSerial.h"
#include "src/core/Logger.h"
#include "src/core/SerialLogBuffer.h"
#include "src/core/StorageManager.h"
#include "src/core/BootState.h"
#include "src/core/DeviceModeManager.h"
#include "src/apps/demo/LedBuzzerApp.h"
#include "src/apps/nfc/NfcReadApp.h"
#include "src/platform/hal/HalNfc.h"
#include "src/apps/system/WebStatusServer.h"
#include "src/platform/hal/HalButtons.h"
#include "src/platform/hal/HalBuzzer.h"
#include "src/platform/hal/HalRgbRing.h"
#include "src/platform/hal/HalWifiLed.h"
#include "src/platform/hal/HalWifiAp.h"
#include "src/platform/hal/WiFiDiagnostics.h"
#include "src/platform/hal/WiFiHardwareTest.h"
#include "src/platform/telemetry/StatusLed.h"
#if APP_USB_HID
#include "src/apps/usb/HidLabApp.h"
#include "src/platform/hal/HalUsbHid.h"
#endif

AppContext gApp;

uint8_t gMenuIndex = 0;
bool gSdReadyLast = false;
bool gPeripheralsReady = false;
uint32_t gPeripheralReadyMs = 0;

bool hackcardPeripheralsReady() {
  return gPeripheralsReady;
}

void initDeferredPeripherals() {
  HcSerial.println("[boot] Starting RGB/buzzer after Wi-Fi settle time...");

  RgbRing.begin(gApp.config.ringBrightness);
  WifiLed.begin(gApp.config.ringBrightness);
  Buzzer.begin(gApp.config.buzzerEnabled);
  Buzzer.setVolume(gApp.config.buzzerVolume);
  Buzzer.setTunes(gApp.config.tuneSuccess, gApp.config.tuneError, gApp.config.tuneBoot, gApp.config.tuneNfc);

  if (gApp.config.bootAnimation) {
    if (gApp.config.bootRingPattern >= 1 && gApp.config.bootRingPattern <= 12) {
      RgbRing.startDemo(gApp.config.bootRingPattern, gApp.config.bootRingR, gApp.config.bootRingG,
                        gApp.config.bootRingB);
    } else {
      Status.setBoot();
    }
    if (gApp.config.buzzerEnabled) {
      Buzzer.playBoot();
    }
  } else {
    Status.setIdle();
  }

  if (Storage.isSdReady()) {
    Status.setStorageTier(true);
  } else {
    Status.setStorageTier(false);
  }

  if (WifiAp.verifyRunning()) {
    Status.setApActive(true);
    Status.setIdle();
    LedBuzzer.applySettings(gApp.config);
    DeviceModeManager::applyFromConfig(gApp);
    HcSerial.println("[boot] Wi-Fi still OK after RGB init");
  } else {
    HcSerial.println("[boot] WARNING: Wi-Fi dropped after RGB init — restarting AP");
    WifiAp.restart();
    Status.setApActive(WifiAp.verifyRunning());
    Status.setError();
  }

  WifiDiagPrintRuntimeReport();

#if APP_NFC_READ
  if (Nfc.begin()) {
    Log.info("nfc", "PN532 ready");
  } else {
    Log.warn("nfc", "PN532 not detected");
  }
#endif

  gPeripheralsReady = true;
}

void handleButtonGesture(ButtonGesture gesture) {
  switch (gesture) {
    case ButtonGesture::Short:
      gMenuIndex = (gMenuIndex + 1) % RGB_RING_COUNT;
      Status.setMenuPointer(gMenuIndex);
      Log.info("boot", "Short press: menu next");
      break;

    case ButtonGesture::Long:
      if (gPeripheralsReady) {
#if APP_USB_HID
        if (HidLab.tryButtonPayload()) {
          Buzzer.playSuccess();
          Log.info("boot", "Long press: HID payload");
          break;
        }
#endif
        Buzzer.playSuccess();
      }
      Log.info("boot", "Long press: confirm");
      break;

    case ButtonGesture::Double:
      if (gPeripheralsReady) {
        Buzzer.playError();
      }
      Log.info("boot", "Double tap: back");
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  SerialLog.begin();

#if APP_USB_HID
  if (HidLab.begin()) {
    HcSerial.println("[boot] USB HID keyboard ready");
    if (UsbHid.isHostConnected()) {
      HcSerial.println("[boot] USB host connected");
    } else {
      HcSerial.println("[boot] USB host waiting (plug into PC)");
    }
  } else {
    HcSerial.print("[boot] USB HID init failed: ");
    HcSerial.println(UsbHid.lastError());
  }
#endif

  Buttons.begin();
  gApp.applyUserConfigDefaults();

  Log.begin(ACTIVITY_LOGGING);
  Log.info("boot", "HackCard firmware " FIRMWARE_VERSION " starting");
  HcSerial.println("[boot] Wi-Fi first — RGB/SD deferred for HackCard PCB");

  Storage.begin();
  Config.begin(gApp);
  HcSerial.print("[boot] Config source: ");
  HcSerial.println(Config.getSourceLabel());
  HcSerial.print("[boot] Profile name: ");
  HcSerial.println(gApp.config.ownerName);

#if WIFI_HARDWARE_TEST_MODE
  HcSerial.println("[boot] === Wi-Fi hardware test mode enabled ===");
  WifiHwTest.runFullTest(WIFI_TEST_STA_SSID, WIFI_TEST_STA_PASSWORD, WIFI_TEST_CONNECT_TIMEOUT);
  HcSerial.println("[boot] Starting AP after hardware test...");
#endif

  if (!WebStatus.begin(&gApp)) {
    Log.warn("web", "Failed to start status AP/web server");
    WifiDiagPrintBootReport();
  } else {
    Log.info("web", "Status dashboard online");
    HcSerial.println();
    HcSerial.println("=== Connect to HackCard ===");
    HcSerial.printf("Wi-Fi SSID : %s\n", gApp.config.apSsid);
    HcSerial.printf("Password   : %s\n", gApp.config.apPassword);
    HcSerial.println("Band       : 2.4 GHz ONLY (not 5 GHz)");
    HcSerial.println("Browser    : http://192.168.4.1/");
    HcSerial.println(WifiAp.getStatusText());
    HcSerial.println("Scan for Wi-Fi NOW — LEDs start after 3 seconds");
    HcSerial.println("===========================");
  }

  gPeripheralReadyMs = millis() + WIFI_PERIPHERAL_DELAY_MS;

  Cli.begin(&gApp, CLI_SERIAL_ENABLED);
  Log.activity("system", "boot", "ok");
  gSdReadyLast = Storage.isSdReady();
}

void loop() {
  if (!gPeripheralsReady && millis() >= gPeripheralReadyMs) {
    if (WifiAp.verifyRunning()) {
      initDeferredPeripherals();
    } else {
      HcSerial.println("[boot] Wi-Fi not ready yet — retrying AP before RGB init");
      WifiAp.restart();
      gPeripheralReadyMs = millis() + 1000;
    }
  }

  Buttons.update();

  if (gPeripheralsReady) {
    RgbRing.update();
    WifiLed.update();
  }

  Storage.loop();

  if (Storage.isSdReady() != gSdReadyLast) {
    gSdReadyLast = Storage.isSdReady();
    if (gPeripheralsReady) {
      Status.setStorageTier(gSdReadyLast);
      Buzzer.playSuccess();
    }
    Log.info("storage", gSdReadyLast ? "SD inserted" : "SD removed");
  }

  const ButtonGesture gesture = Buttons.consumeGesture();
  if (gesture != ButtonGesture::None) {
    handleButtonGesture(gesture);
  }

#if CLI_SERIAL_ENABLED
  while (Serial.available() > 0) {
    Cli.feed(static_cast<char>(Serial.read()));
  }
#endif

  WebStatus.loop();
}
