#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "../Config.h"

#define INDOOR_EXT_DESC_MAX 48

// Estado compartido: telemetría MQTT + red + displays.
struct IndoorData {
  float temperature = WEATHER_VALUE_INVALID;
  float humidity = WEATHER_VALUE_INVALID;
  float pressure = WEATHER_VALUE_INVALID;
  float altitude = WEATHER_VALUE_INVALID;
  float extTemp = WEATHER_VALUE_INVALID;
  float extHumidity = WEATHER_VALUE_INVALID;
  bool bmp180OK = false;
  bool aht10OK = false;
  bool extOK = false;
  char extDesc[INDOOR_EXT_DESC_MAX];
  char outdoorIp[16];
  char outdoorFirmware[16];

  bool wifiConnected = false;
  int32_t wifiRSSI = 0;
  IPAddress ipAddress;
  bool mqttConnected = false;
  bool mqttHasData = false;
  unsigned long lastMqttMs = 0;
  unsigned long uptime = 0;
  uint32_t freeHeap = 0;

  bool max7219OK = false;
  bool oledOK = false;
  bool lcdOK = false;

  bool dataIsFresh() const {
    return mqttHasData && (millis() - lastMqttMs) < MQTT_STALE_MS;
  }

  static IndoorData& instance();
};
