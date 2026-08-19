#pragma once

#include "../core/WeatherData.h"

class AHT10Sensor {
public:
  bool begin();
  void read(WeatherData& data);

private:
  bool _initialized;
  bool softReset();
  bool initializeSensor();
  bool triggerMeasurement();
  bool readRaw(uint32_t& humidityRaw, uint32_t& temperatureRaw);
};
