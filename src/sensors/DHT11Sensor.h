#pragma once

#include "../core/WeatherData.h"

class DHT11Sensor {
public:
  bool begin();
  void read(WeatherData& data);

private:
  bool _initialized;
  bool readSample(int& humidity, int& temperature);
};
