#include "SensorManager.h"

bool SensorManager::begin() {
  bool bmp = _bmp180.begin();
  bool dht = _dht11.begin();

  WeatherData& data = WeatherData::instance();
  data.bmp180OK = bmp;
  data.dht11OK = dht;

  return bmp || dht;
}

void SensorManager::readBMP180() {
  _bmp180.read(WeatherData::instance());
}

void SensorManager::readDHT11() {
  _dht11.read(WeatherData::instance());
}
