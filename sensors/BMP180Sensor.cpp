#include "BMP180Sensor.h"
#include "../utils/I2CBus.h"
#include "../Config.h"

bool BMP180Sensor::begin() {
  I2CBus::beginBMP();
  _initialized = _sensor.begin(BMP085_STANDARD);
  return _initialized;
}

void BMP180Sensor::read(WeatherData& data) {
  if (!_initialized) {
    data.bmp180OK = false;
    return;
  }

  I2CBus::beginBMP();

  float temp = _sensor.readTemperature();
  int32_t pressurePa = _sensor.readPressure();

  if (pressurePa <= 0) {
    data.bmp180OK = false;
    return;
  }

  float pressureHpa = pressurePa / 100.0f;
  float altitude = _sensor.readAltitude(SEA_LEVEL_PRESSURE_HPA * 100.0f);

  if (temp < -40.0f || temp > 85.0f) {
    data.bmp180OK = false;
    return;
  }

  if (pressureHpa < 300.0f || pressureHpa > 1100.0f) {
    data.bmp180OK = false;
    return;
  }

  data.temperatureBMP = temp;
  data.pressure = pressureHpa;
  data.altitude = altitude;
  data.bmp180OK = true;
  data.lastBMPReading = millis();
  data.updateDerivedValues();
}
