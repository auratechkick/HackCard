#include "WifiCaptivePortal.h"

#include "../../core/HcSerial.h"



#include <DNSServer.h>

#include <WiFi.h>



WifiCaptivePortal WifiCaptive;



namespace {



DNSServer sDns;

constexpr uint16_t kDnsPort = 53;



}  // namespace



void WifiCaptivePortal::startDns() {

  if (_running) {

    return;

  }



  const IPAddress apIp = WiFi.softAPIP();

  if (apIp == IPAddress(0, 0, 0, 0)) {

    return;

  }



  sDns.start(kDnsPort, "*", apIp);

  _running = true;

  HcSerial.printf("[portal] DNS captive active -> %s\n", apIp.toString().c_str());

}



void WifiCaptivePortal::stopDns() {

  if (!_running) {

    return;

  }



  sDns.stop();

  _running = false;

  HcSerial.println("[portal] DNS captive stopped");

}



void WifiCaptivePortal::begin(bool enabled) {

  _enabled = enabled;

  if (_enabled) {

    startDns();

  } else {

    stopDns();

  }

}



void WifiCaptivePortal::setEnabled(bool enabled) {

  if (_enabled == enabled) {

    return;

  }



  _enabled = enabled;

  if (_enabled) {

    startDns();

  } else {

    stopDns();

  }

}



void WifiCaptivePortal::loop() {

  if (!_enabled) {

    return;

  }



  if (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {

    if (_running) {

      stopDns();

    }

    return;

  }



  if (!_running) {

    startDns();

  }



  sDns.processNextRequest();

}


