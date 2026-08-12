#include "WeatherData.h"
#include "../Config.h"

WeatherData WeatherData::_instance;

WeatherData::WeatherData()
  : temperatureBMP(WEATHER_VALUE_INVALID),
    temperatureDHT(WEATHER_VALUE_INVALID),
    temperatureMain(WEATHER_VALUE_INVALID),
    humidity(WEATHER_VALUE_INVALID),
    pressure(WEATHER_VALUE_INVALID),
    altitude(WEATHER_VALUE_INVALID),
    bmp180OK(false),
    dht11OK(false),
    max7219OK(false),
    oledOK(false),
    ntpSynced(false),
    lastBMPReading(0),
    lastDHTReading(0),
    wifiConnected(false),
    wifiRSSI(0),
    hostname(HOSTNAME),
    uptime(0),
    firmwareVersion(FIRMWARE_VERSION),
    otaInProgress(false),
    otaProgress(0),
    freeHeap(0) {}

WeatherData& WeatherData::instance() {
  return _instance;
}

void WeatherData::init() {
  temperatureBMP = WEATHER_VALUE_INVALID;
  temperatureDHT = WEATHER_VALUE_INVALID;
  temperatureMain = WEATHER_VALUE_INVALID;
  humidity = WEATHER_VALUE_INVALID;
  pressure = WEATHER_VALUE_INVALID;
  altitude = WEATHER_VALUE_INVALID;
  bmp180OK = false;
  dht11OK = false;
  max7219OK = false;
  oledOK = false;
  ntpSynced = false;
  wifiConnected = false;
  wifiRSSI = 0;
  otaInProgress = false;
  otaProgress = 0;
  updateDerivedValues();
}

bool weatherValueIsValid(float value) {
  return value > (WEATHER_VALUE_INVALID + 1.0f);
}

void WeatherData::updateDerivedValues() {
  if (dht11OK && weatherValueIsValid(temperatureDHT)) {
    temperatureMain = temperatureDHT;
  } else if (bmp180OK && weatherValueIsValid(temperatureBMP)) {
    temperatureMain = temperatureBMP;
  } else {
    temperatureMain = WEATHER_VALUE_INVALID;
  }
}
