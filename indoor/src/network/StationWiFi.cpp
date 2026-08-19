#include "StationWiFi.h"
#include "../Config.h"
#include "../core/IndoorData.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

void StationWiFi::begin() {
  // Prints antes de autoConnect (puede bloquear hasta timeout del portal).
  Serial.println(F("[WIFI] Preparando WiFiManager..."));
  Serial.print(F("[WIFI] Hostname: "));
  Serial.println(HOSTNAME);
  Serial.print(F("[WIFI] Connect timeout "));
  Serial.print(WIFI_CONNECT_TIMEOUT_SEC);
  Serial.print(F(" s, portal timeout "));
  Serial.print(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);
  Serial.println(F(" s"));
  Serial.flush();

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
  Serial.println(F("[WIFI] Abrí http://192.168.4.1"));
  Serial.flush();

  const bool connected = wm.autoConnect(HOSTNAME);
  if (connected) {
    Serial.print(F("[WIFI] Conectado. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("[WIFI] Sin conexión tras portal/timeout; reintento en background"));
  }

  _lastAttempt = millis();
}

void StationWiFi::update() {
  IndoorData& data = IndoorData::instance();
  const bool connected = (WiFi.status() == WL_CONNECTED);

  data.wifiConnected = connected;
  if (connected) {
    data.wifiRSSI = WiFi.RSSI();
    data.ipAddress = WiFi.localIP();
    return;
  }

  data.wifiRSSI = 0;
  if (millis() - _lastAttempt >= WIFI_INTERVAL_MS) {
    Serial.println(F("[WIFI] Reintentando con credenciales guardadas..."));
    WiFi.disconnect();
    WiFi.begin();
    _lastAttempt = millis();
  }
}
