#pragma once

#include "../core/IndoorData.h"

// OLED SSD1306 opcional (OLED_ENABLED en Config.h). Placa TTGO sin OLED → stubs.
class OledDisplay {
public:
  static bool begin();
  static void showBoot(const char* line1, const char* line2);
  static void showStatus(const IndoorData& data);

private:
  static bool _ready;
  static unsigned long _lastUpdate;
};
