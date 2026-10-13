#pragma once



#include <Arduino.h>



class WifiCaptivePortal {

public:

  void begin(bool enabled);

  void setEnabled(bool enabled);

  void loop();

  bool isEnabled() const { return _enabled; }

  bool isRunning() const { return _running; }



private:

  void startDns();

  void stopDns();



  bool _enabled = false;

  bool _running = false;

};



extern WifiCaptivePortal WifiCaptive;


