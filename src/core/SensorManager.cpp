#include "SensorManager.h"

bool SensorManager::begin() {
  bool bmp = _bmp180.begin();
  bool aht = _aht10.begin();

  WeatherData& data = WeatherData::instance();
  data.bmp180OK = bmp;
  data.aht10OK = aht;

  return bmp || aht;
}

void SensorManager::readBMP180() {
  _bmp180.read(WeatherData::instance());
}

void SensorManager::readAHT10() {
  _aht10.read(WeatherData::instance());
}
