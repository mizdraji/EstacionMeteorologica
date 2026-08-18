#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../Config.h"

struct WeatherData {
  float temperatureBMP;
  float temperatureAHT;
  float temperatureMain;
  float humidity;
  float pressure;
  float altitude;

  bool bmp180OK;
  bool aht10OK;
  bool max7219OK;
  bool oledOK;
  bool ntpSynced;

  bool wifiConnected;
  int wifiRSSI;
  IPAddress ipAddress;
  unsigned long uptime;
  const char* firmwareVersion;

  bool otaInProgress;
  uint8_t otaProgress;

  uint32_t freeHeap;

  // Referencia OpenWeatherMap (current weather)
  float externalTemperature;
  float externalHumidity;
  char externalDescription[EXTERNAL_DESC_MAX];
  bool externalOK;
  unsigned long externalLastUpdateMs;

  void init();
  void updateDerivedValues();
  static WeatherData& instance();

private:
  WeatherData();
  static WeatherData _instance;
};

bool weatherValueIsValid(float value);
