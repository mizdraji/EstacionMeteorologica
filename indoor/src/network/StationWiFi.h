#pragma once

#include <WiFiManager.h>

class StationWiFi {
public:
  void begin();
  void update();

private:
  WiFiManager _wm;
  unsigned long _lastAttempt = 0;
  bool _portalRunning = false;
};
