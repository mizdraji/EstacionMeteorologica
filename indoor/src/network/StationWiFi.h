#pragma once

class StationWiFi {
public:
  void begin();
  void update();

private:
  unsigned long _lastAttempt = 0;
};
