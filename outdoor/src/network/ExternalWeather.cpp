#include "ExternalWeather.h"
#include "../Config.h"
#include "../core/WeatherData.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <string.h>

#ifndef OPENWEATHERMAP_API_KEY
#define OPENWEATHERMAP_API_KEY ""
#endif

static unsigned long _lastAttemptMs = 0;
static bool _started = false;

static bool apiKeyConfigured() {
  return OPENWEATHERMAP_API_KEY[0] != '\0';
}

static void markFail() {
  WeatherData& data = WeatherData::instance();
  data.externalOK = false;
  data.externalTemperature = WEATHER_VALUE_INVALID;
  data.externalHumidity = WEATHER_VALUE_INVALID;
  data.externalDescription[0] = '\0';
  data.externalLastUpdateMs = millis();
}

static bool buildUrl(String& url) {
  if (!apiKeyConfigured()) {
    return false;
  }

  url = F("http://api.openweathermap.org/data/2.5/weather?");
#if OWM_USE_CITY_ID
  url += F("id=");
  url += String(OWM_CITY_ID);
#else
  url += F("lat=");
  url += String(static_cast<double>(OWM_LAT), 14);
  url += F("&lon=");
  url += String(static_cast<double>(OWM_LON), 14);
#endif
  url += F("&units=metric&lang=es&appid=");
  url += OPENWEATHERMAP_API_KEY;
  return true;
}

static bool parsePayload(const String& payload) {
  // Filtro liviano: solo campos usados (heap ESP8266).
  StaticJsonDocument<192> filter;
  filter["main"]["temp"] = true;
  filter["main"]["humidity"] = true;
  filter["weather"][0]["description"] = true;
  filter["name"] = true;

  StaticJsonDocument<384> doc;
  const DeserializationError err =
      deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (err) {
    Serial.print(F("[OWM] JSON error: "));
    Serial.println(err.c_str());
    return false;
  }

  if (!doc["main"]["temp"].is<float>() && !doc["main"]["temp"].is<int>()) {
    Serial.println(F("[OWM] missing temp"));
    return false;
  }

  WeatherData& data = WeatherData::instance();
  data.externalTemperature = doc["main"]["temp"].as<float>();
  data.externalHumidity = doc["main"]["humidity"].as<float>();

  const char* desc = doc["weather"][0]["description"] | "";
  strncpy(data.externalDescription, desc, EXTERNAL_DESC_MAX - 1);
  data.externalDescription[EXTERNAL_DESC_MAX - 1] = '\0';

  data.externalOK = true;
  data.externalLastUpdateMs = millis();
  return true;
}

static void fetchOnce() {
  WeatherData& data = WeatherData::instance();
  if (!data.wifiConnected || WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!apiKeyConfigured()) {
    markFail();
    strncpy(data.externalDescription, "sin API key", EXTERNAL_DESC_MAX - 1);
    data.externalDescription[EXTERNAL_DESC_MAX - 1] = '\0';
    return;
  }

  String url;
  if (!buildUrl(url)) {
    markFail();
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(8000);
  http.setReuse(false);

  if (!http.begin(client, url)) {
    Serial.println(F("[OWM] http.begin failed"));
    markFail();
    return;
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.print(F("[OWM] HTTP "));
    Serial.println(code);
    http.end();
    markFail();
    return;
  }

  const String payload = http.getString();
  http.end();

  if (!parsePayload(payload)) {
    markFail();
    return;
  }

  Serial.print(F("[OWM] OK "));
  Serial.print(data.externalTemperature, 1);
  Serial.print(F(" C / "));
  Serial.print(data.externalHumidity, 0);
  Serial.print(F("% / "));
  Serial.println(data.externalDescription);
}

void ExternalWeather::begin() {
  _started = true;
  _lastAttemptMs = 0;
  WeatherData& data = WeatherData::instance();
  data.externalOK = false;
  data.externalTemperature = WEATHER_VALUE_INVALID;
  data.externalHumidity = WEATHER_VALUE_INVALID;
  data.externalDescription[0] = '\0';
  data.externalLastUpdateMs = 0;
}

void ExternalWeather::update() {
  if (!_started) {
    return;
  }

  const unsigned long now = millis();
  // Primera consulta pronto tras WiFi; luego cada OWM_INTERVAL_MS.
  const unsigned long interval =
      (_lastAttemptMs == 0) ? 15000UL : static_cast<unsigned long>(OWM_INTERVAL_MS);

  if (_lastAttemptMs != 0 && (now - _lastAttemptMs) < interval) {
    return;
  }

  // Solo avanzar el timer si hay WiFi (o si ya pasó el intervalo con WiFi).
  WeatherData& data = WeatherData::instance();
  if (!data.wifiConnected) {
    return;
  }

  _lastAttemptMs = now;
  fetchOnce();
}
