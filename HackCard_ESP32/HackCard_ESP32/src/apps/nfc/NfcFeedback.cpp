#include "NfcFeedback.h"

#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/telemetry/StatusLed.h"

void NfcFeedback::scanStart() {
  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
}

void NfcFeedback::success() {
  Status.setSuccess();
  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
}

void NfcFeedback::error() {
  Status.setError();
  if (Buzzer.isEnabled()) {
    Buzzer.playError();
  }
}

void NfcFeedback::writeOk() {
  Status.setSuccess();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcWriteOk();
  }
}

void NfcFeedback::cloneOk() {
  Status.setSuccess();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcCloneOk();
  }
}

void NfcFeedback::killOk() {
  Status.setSuccess();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcKillOk();
  }
}
