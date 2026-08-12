#pragma once

#include "../../core/WeatherData.h"

class OLEDDisplayModule {
public:
  static bool begin();
  static void showDiagnostics(const WeatherData& data);
  static void showOta(uint8_t progress);
  static void showBootMessage(const char* line1, const char* line2);

private:
  static bool _ready;
  static unsigned long _lastUpdate;
};
