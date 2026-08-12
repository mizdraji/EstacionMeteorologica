#include "SerialDiagnostics.h"
#include "../Config.h"
#include "../core/WeatherData.h"

void SerialDiagnostics::printBootBanner() {
  const WeatherData& data = WeatherData::instance();

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F(" WEATHER STATION"));
  Serial.println(F("========================================"));
  Serial.print(F("Firmware: "));
  Serial.println(FIRMWARE_VERSION);
  Serial.println();
  Serial.print(F("BMP180: "));
  Serial.println(data.bmp180OK ? F("OK") : F("FAIL"));
  Serial.print(F("DHT11: "));
  Serial.println(data.dht11OK ? F("OK") : F("FAIL"));
  Serial.print(F("MAX7219: "));
  Serial.println(data.max7219OK ? F("OK") : F("FAIL"));
  Serial.print(F("OLED: "));
  Serial.println(data.oledOK ? F("OK") : F("FAIL"));
  Serial.println();
  Serial.print(F("Hostname: "));
  Serial.println(HOSTNAME);
  Serial.print(F("NTP: "));
  Serial.println(NTP_SERVER);
  Serial.println(F("========================================"));
}

void SerialDiagnostics::printPeriodicStatus() {
  const WeatherData& data = WeatherData::instance();

  if (!data.wifiConnected) {
    Serial.println(F("[SYS] WiFi desconectado, sensores activos"));
    return;
  }

  Serial.print(F("[SYS] IP: "));
  Serial.print(data.ipAddress);
  Serial.print(F(" RSSI: "));
  Serial.print(data.wifiRSSI);
  Serial.print(F(" Heap: "));
  Serial.print(data.freeHeap);
  Serial.print(F(" NTP: "));
  Serial.println(data.ntpSynced ? F("OK") : F("SYNC..."));
}
