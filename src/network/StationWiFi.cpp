#include "StationWiFi.h"
#include "../Config.h"
#include "WebServerManager.h"
#include "OTAManager.h"
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

void StationWiFi::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);

  WiFiManager wm;
  wm.setDebugOutput(false);
  wm.setHostname(HOSTNAME);
  wm.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SEC);

  Serial.println(F("[WIFI] Intentando credenciales guardadas..."));
  Serial.print(F("[WIFI] Si falla, portal AP: "));
  Serial.println(HOSTNAME);
  Serial.print(F("[WIFI] Abrí http://192.168.4.1 (timeout "));
  Serial.print(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);
  Serial.println(F(" s)"));

  // Persiste SSID/pass en flash del SDK WiFi (no usa LittleFS).
  bool connected = wm.autoConnect(HOSTNAME);

  if (connected) {
    Serial.print(F("[WIFI] Conectado. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("[WIFI] Sin conexión tras portal/timeout; reintento en background"));
  }

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
      Serial.println(F("[WIFI] Servicios web/OTA/mDNS iniciados"));
    }
    return;
  }

  data.wifiRSSI = 0;
  if (_servicesStarted) {
    WebServerManager::shutdown();
    OTAManager::shutdown();
    _servicesStarted = false;
    Serial.println(F("[WIFI] Desconectado; servicios detenidos"));
  }

  if (millis() - _lastAttempt >= WIFI_INTERVAL_MS) {
    Serial.println(F("[WIFI] Reintentando con credenciales guardadas..."));
    WiFi.disconnect();
    WiFi.begin();  // usa SSID/pass persistidos por WiFiManager
    _lastAttempt = millis();
  }
}
