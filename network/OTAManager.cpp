#include "OTAManager.h"
#include "../Config.h"
#include "../core/WeatherData.h"
#include <ArduinoOTA.h>

static bool _otaReady = false;

void OTAManager::begin() {
  if (_otaReady) {
    return;
  }

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    WeatherData::instance().otaInProgress = true;
    WeatherData::instance().otaProgress = 0;
    Serial.println("[OTA] Inicio");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    uint8_t percent = (progress * 100) / total;
    WeatherData::instance().otaProgress = percent;
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Completado");
    WeatherData::instance().otaInProgress = false;
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error %u\n", error);
    WeatherData::instance().otaInProgress = false;
  });

  ArduinoOTA.begin();
  _otaReady = true;
}

void OTAManager::handle() {
  if (_otaReady) {
    ArduinoOTA.handle();
  }
}

void OTAManager::shutdown() {
  _otaReady = false;
}
