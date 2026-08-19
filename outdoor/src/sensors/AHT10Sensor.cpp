#include "AHT10Sensor.h"
#include "../utils/I2CBus.h"
#include "../Config.h"
#include <Wire.h>

namespace {
constexpr uint8_t CMD_SOFT_RESET = 0xBA;
constexpr uint8_t CMD_INITIALIZE = 0xBE;
constexpr uint8_t CMD_TRIGGER = 0xAC;
constexpr uint8_t STATUS_BUSY = 0x80;
}

bool AHT10Sensor::begin() {
  I2CBus::beginBMP();
  delay(40);

  if (!softReset()) {
    _initialized = false;
    return false;
  }
  delay(20);

  if (!initializeSensor()) {
    _initialized = false;
    return false;
  }
  delay(10);

  _initialized = true;
  return true;
}

bool AHT10Sensor::softReset() {
  Wire.beginTransmission(AHT10_I2C_ADDR);
  Wire.write(CMD_SOFT_RESET);
  return Wire.endTransmission() == 0;
}

bool AHT10Sensor::initializeSensor() {
  Wire.beginTransmission(AHT10_I2C_ADDR);
  Wire.write(CMD_INITIALIZE);
  Wire.write(0x08);
  Wire.write(0x00);
  return Wire.endTransmission() == 0;
}

bool AHT10Sensor::triggerMeasurement() {
  Wire.beginTransmission(AHT10_I2C_ADDR);
  Wire.write(CMD_TRIGGER);
  Wire.write(0x33);
  Wire.write(0x00);
  return Wire.endTransmission() == 0;
}

bool AHT10Sensor::readRaw(uint32_t& humidityRaw, uint32_t& temperatureRaw) {
  if (!triggerMeasurement()) {
    return false;
  }

  delay(80);

  for (uint8_t attempt = 0; attempt < 5; attempt++) {
    if (Wire.requestFrom(static_cast<uint8_t>(AHT10_I2C_ADDR), static_cast<uint8_t>(6)) != 6) {
      delay(10);
      continue;
    }

    uint8_t data[6];
    for (uint8_t i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }

    if (data[0] & STATUS_BUSY) {
      delay(10);
      continue;
    }

    humidityRaw = (static_cast<uint32_t>(data[1]) << 12) |
                  (static_cast<uint32_t>(data[2]) << 4) |
                  (static_cast<uint32_t>(data[3]) >> 4);
    temperatureRaw = ((static_cast<uint32_t>(data[3]) & 0x0FUL) << 16) |
                     (static_cast<uint32_t>(data[4]) << 8) |
                     static_cast<uint32_t>(data[5]);
    return true;
  }

  return false;
}

void AHT10Sensor::read(WeatherData& data) {
  if (!_initialized) {
    data.aht10OK = false;
    data.updateDerivedValues();
    return;
  }

  I2CBus::beginBMP();

  uint32_t humidityRaw = 0;
  uint32_t temperatureRaw = 0;
  if (!readRaw(humidityRaw, temperatureRaw)) {
    data.aht10OK = false;
    data.updateDerivedValues();
    return;
  }

  float humidity = (humidityRaw * 100.0f) / 1048576.0f;
  float temperature = ((temperatureRaw * 200.0f) / 1048576.0f) - 50.0f;

  if (humidity < 0.0f || humidity > 100.0f || temperature < -40.0f || temperature > 85.0f) {
    data.aht10OK = false;
    data.updateDerivedValues();
    return;
  }

  data.temperatureAHT = temperature;
  data.humidity = humidity;
  data.aht10OK = true;
  data.updateDerivedValues();
}
