#pragma once

#include "../core/WeatherData.h"
#include "Max7219Display.h"

class DisplayManager {
public:
  bool begin();
  void update();

private:
  Max7219Display _max7219;

  void renderClock(const WeatherData& data);
};
