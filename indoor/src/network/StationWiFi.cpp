#include "StationWiFi.h"
#include "../Config.h"
#include "../core/IndoorData.h"
#include <WiFi.h>

void StationWiFi::begin() {
  Serial.println(F("[WIFI] Preparando WiFiManager (ESP32)..."));
  Serial.print(F("[WIFI] AP / hostname: "));
  Serial.println(HOSTNAME);
  Serial.print(F("[WIFI] Connect timeout "));
  Serial.print(WIFI_CONNECT_TIMEOUT_SEC);
  Serial.print(F(" s, portal timeout "));
  Serial.print(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);
  Serial.println(F(" s"));
  Serial.println(F("[WIFI] Buscá la red WiFi del ESP y abrí http://192.168.4.1"));
  Serial.flush();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setHostname(HOSTNAME);

  _wm.setDebugOutput(true);
  _wm.setHostname(HOSTNAME);
  _wm.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);
  _wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SEC);
  // Portal no bloqueante: el AP sigue visible mientras corre el loop.
  _wm.setConfigPortalBlocking(false);

  Serial.println(F("[WIFI] autoConnect (credenciales guardadas o portal AP)..."));
  Serial.flush();

  const bool connected = _wm.autoConnect(HOSTNAME);
  _portalRunning = _wm.getConfigPortalActive();

  if (connected) {
    Serial.print(F("[WIFI] Conectado. IP: "));
    Serial.println(WiFi.localIP());
    _portalRunning = false;
  } else if (_portalRunning) {
    Serial.print(F("[WIFI] Portal AP activo: "));
    Serial.println(HOSTNAME);
    Serial.println(F("[WIFI] Conectate a ese AP → http://192.168.4.1"));
  } else {
    Serial.println(F("[WIFI] Sin conexión; arrancando portal AP..."));
    _wm.startConfigPortal(HOSTNAME);
    _portalRunning = true;
    Serial.print(F("[WIFI] AP: "));
    Serial.println(HOSTNAME);
  }

  _lastAttempt = millis();
}

void StationWiFi::update() {
  // Necesario con portal no bloqueante.
  _wm.process();
  _portalRunning = _wm.getConfigPortalActive();

  IndoorData& data = IndoorData::instance();
  const bool connected = (WiFi.status() == WL_CONNECTED);

  data.wifiConnected = connected;
  if (connected) {
    data.wifiRSSI = WiFi.RSSI();
    data.ipAddress = WiFi.localIP();
    return;
  }

  data.wifiRSSI = 0;

  // Si el portal se cerró y seguimos offline, reabrirlo.
  if (!_portalRunning && (millis() - _lastAttempt >= WIFI_INTERVAL_MS * 6UL)) {
    Serial.println(F("[WIFI] Offline → reabriendo portal AP..."));
    Serial.print(F("[WIFI] AP: "));
    Serial.println(HOSTNAME);
    WiFi.mode(WIFI_AP_STA);
    _wm.setConfigPortalBlocking(false);
    _wm.startConfigPortal(HOSTNAME);
    _portalRunning = true;
    _lastAttempt = millis();
    return;
  }

  if (!_portalRunning && (millis() - _lastAttempt >= WIFI_INTERVAL_MS)) {
    Serial.println(F("[WIFI] Reintentando credenciales guardadas..."));
    WiFi.begin();
    _lastAttempt = millis();
  }
}
