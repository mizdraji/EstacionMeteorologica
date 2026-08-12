#include "StationWiFi.h"
#include "../Config.h"
#include "WebServerManager.h"
#include "OTAManager.h"
#include <ESP8266mDNS.h>

void StationWiFi::begin() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  _lastAttempt = millis();
  _servicesStarted = false;
}

void StationWiFi::update() {
  WeatherData& data = WeatherData::instance();
  bool connected = (WiFi.status() == WL_CONNECTED);

  data.wifiConnected = connected;

  if (connected) {
    data.wifiRSSI = WiFi.RSSI();
    data.ipAddress = WiFi.localIP();

    if (!_servicesStarted) {
      if (MDNS.begin(HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
      }
      WebServerManager::begin();
      OTAManager::begin();
      _servicesStarted = true;
    }
    return;
  }

  data.wifiRSSI = 0;
  if (_servicesStarted) {
    WebServerManager::shutdown();
    OTAManager::shutdown();
    _servicesStarted = false;
  }

  if (millis() - _lastAttempt >= WIFI_INTERVAL_MS) {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    _lastAttempt = millis();
  }
}
