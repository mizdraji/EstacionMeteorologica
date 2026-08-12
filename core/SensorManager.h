#pragma once

#include "../core/WeatherData.h"
#include "../sensors/BMP180Sensor.h"
#include "../sensors/DHT11Sensor.h"

class SensorManager {
public:
  bool begin();
  void readBMP180();
  void readDHT11();

private:
  BMP180Sensor _bmp180;
  DHT11Sensor _dht11;
};
