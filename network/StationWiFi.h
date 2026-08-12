#pragma once

#include <ESP8266WiFi.h>
#include "../core/WeatherData.h"

class StationWiFi {
public:
  void begin();
  void update();

private:
  unsigned long _lastAttempt;
  bool _servicesStarted;
};
