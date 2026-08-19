#pragma once

#include "../core/WeatherData.h"
#include "../sensors/BMP180Sensor.h"
#include "../sensors/AHT10Sensor.h"

class SensorManager {
public:
  bool begin();
  void readBMP180();
  void readAHT10();

private:
  BMP180Sensor _bmp180;
  AHT10Sensor _aht10;
};
