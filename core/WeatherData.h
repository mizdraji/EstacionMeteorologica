#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

struct WeatherData {
  float temperatureBMP;
  float temperatureDHT;
  float temperatureMain;
  float humidity;
  float pressure;
  float altitude;

  bool bmp180OK;
  bool dht11OK;
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

  void init();
  void updateDerivedValues();
  static WeatherData& instance();

private:
  WeatherData();
  static WeatherData _instance;
};

bool weatherValueIsValid(float value);
