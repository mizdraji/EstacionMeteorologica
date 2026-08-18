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
  Serial.print(F("AHT10: "));
  Serial.println(data.aht10OK ? F("OK") : F("FAIL"));
  Serial.print(F("MAX7219: "));
  Serial.println(data.max7219OK ? F("OK") : F("FAIL"));
  Serial.print(F("OLED: "));
  Serial.println(data.oledOK ? F("OK") : F("FAIL"));
  Serial.println();
  Serial.print(F("Hostname: "));
  Serial.println(HOSTNAME);
  Serial.println(F("WiFi: WiFiManager (portal AP si hace falta)"));
  Serial.print(F("NTP: "));
  Serial.println(NTP_SERVER);
  Serial.print(F("OWM: cada "));
  Serial.print(OWM_INTERVAL_MS / 60000UL);
  Serial.println(F(" min"));
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
  Serial.print(data.ntpSynced ? F("OK") : F("SYNC..."));
  Serial.print(F(" OWM: "));
  Serial.println(data.externalOK ? F("OK") : F("N/A"));
}
