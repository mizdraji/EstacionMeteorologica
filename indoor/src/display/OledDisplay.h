#pragma once

#include "../core/IndoorData.h"

class OledDisplay {
public:
  static bool begin();
  static void showBoot(const char* line1, const char* line2);
  static void showStatus(const IndoorData& data);

private:
  static bool _ready;
  static unsigned long _lastUpdate;
};
