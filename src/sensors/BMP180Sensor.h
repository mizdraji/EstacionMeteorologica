#pragma once

#include "BMP085.h"
#include "../core/WeatherData.h"

class BMP180Sensor {
public:
  bool begin();
  void read(WeatherData& data);

private:
  Adafruit_BMP085 _sensor;
  bool _initialized;
};
