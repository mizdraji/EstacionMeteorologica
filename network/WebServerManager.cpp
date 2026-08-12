#include "WebServerManager.h"
#include "../Config.h"
#include "../core/WeatherData.h"
#include <ESP8266WebServer.h>
#include <LittleFS.h>

static ESP8266WebServer server(WEB_SERVER_PORT);
static bool _webReady = false;

static String jsonFloatOrNull(float value) {
  if (!weatherValueIsValid(value)) {
    return "null";
  }
  return String(value, 1);
}

static void handleApiData() {
  const WeatherData& data = WeatherData::instance();
  String json = "{";
  json += "\"temperature_bmp\":" + jsonFloatOrNull(data.temperatureBMP) + ",";
  json += "\"temperature_dht\":" + jsonFloatOrNull(data.temperatureDHT) + ",";
  json += "\"temperature_main\":" + jsonFloatOrNull(data.temperatureMain) + ",";
  json += "\"humidity\":" + jsonFloatOrNull(data.humidity) + ",";
  json += "\"pressure\":" + jsonFloatOrNull(data.pressure) + ",";
  json += "\"altitude\":" + jsonFloatOrNull(data.altitude) + ",";
  json += "\"bmp180_ok\":" + String(data.bmp180OK ? "true" : "false") + ",";
  json += "\"dht11_ok\":" + String(data.dht11OK ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

static void handleApiStatus() {
  const WeatherData& data = WeatherData::instance();
  String json = "{";
  json += "\"wifi_connected\":" + String(data.wifiConnected ? "true" : "false") + ",";
  json += "\"rssi\":" + String(data.wifiRSSI) + ",";
  json += "\"ip\":\"" + data.ipAddress.toString() + "\",";
  json += "\"hostname\":\"" + String(HOSTNAME) + "\",";
  json += "\"uptime\":" + String(data.uptime) + ",";
  json += "\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",";
  json += "\"free_heap\":" + String(data.freeHeap) + ",";
  json += "\"bmp180_ok\":" + String(data.bmp180OK ? "true" : "false") + ",";
  json += "\"dht11_ok\":" + String(data.dht11OK ? "true" : "false") + ",";
  json += "\"max7219_ok\":" + String(data.max7219OK ? "true" : "false") + ",";
  json += "\"ntp_synced\":" + String(data.ntpSynced ? "true" : "false") + ",";
  json += "\"ota_in_progress\":" + String(data.otaInProgress ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

static void handleNotFound() {
  if (LittleFS.exists("/index.html")) {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
    return;
  }
  server.send(404, "text/plain", "Not found");
}

void WebServerManager::begin() {
  if (_webReady) {
    return;
  }

  if (!LittleFS.begin()) {
    Serial.println("[WEB] LittleFS mount failed");
  }

  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/status", HTTP_GET, handleApiStatus);

  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/script.js", LittleFS, "/script.js");
  server.serveStatic("/", LittleFS, "/index.html");

  server.onNotFound(handleNotFound);
  server.begin();
  _webReady = true;
  Serial.println("[WEB] Server ready");
}

void WebServerManager::handle() {
  if (_webReady) {
    server.handleClient();
  }
}

void WebServerManager::shutdown() {
  _webReady = false;
}
