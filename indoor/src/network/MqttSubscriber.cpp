#include "MqttSubscriber.h"
#include "../Config.h"
#include "../core/IndoorData.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

static char clientId[40];
static unsigned long nextRetryMs = 0;
static unsigned long backoffMs = 1000;
static bool loggedDisconnect = false;

static float jsonFloatOrInvalid(JsonVariantConst v) {
  if (v.isNull()) {
    return WEATHER_VALUE_INVALID;
  }
  if (v.is<float>() || v.is<double>() || v.is<int>() || v.is<long>()) {
    return v.as<float>();
  }
  return WEATHER_VALUE_INVALID;
}

static void applyPayload(const char* payload, unsigned int length) {
  StaticJsonDocument<512> doc;
  const DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.print(F("[MQTT] JSON err: "));
    Serial.println(err.c_str());
    return;
  }

  IndoorData& d = IndoorData::instance();

  // Preferir temp_main; fallback temp_aht / temp_bmp
  if (!doc["temp_main"].isNull()) {
    d.temperature = jsonFloatOrInvalid(doc["temp_main"]);
  } else if (!doc["temp_aht"].isNull()) {
    d.temperature = jsonFloatOrInvalid(doc["temp_aht"]);
  } else {
    d.temperature = jsonFloatOrInvalid(doc["temp_bmp"]);
  }

  d.humidity = jsonFloatOrInvalid(doc["humidity"]);
  d.pressure = jsonFloatOrInvalid(doc["pressure"]);
  d.altitude = jsonFloatOrInvalid(doc["altitude"]);
  d.extTemp = jsonFloatOrInvalid(doc["ext_temp"]);
  d.extHumidity = jsonFloatOrInvalid(doc["ext_humidity"]);

  d.bmp180OK = doc["bmp180_ok"] | false;
  d.aht10OK = doc["aht10_ok"] | false;
  d.extOK = doc["ext_ok"] | false;

  d.extDesc[0] = '\0';
  if (doc["ext_desc"].is<const char*>()) {
    strncpy(d.extDesc, doc["ext_desc"] | "", INDOOR_EXT_DESC_MAX - 1);
    d.extDesc[INDOOR_EXT_DESC_MAX - 1] = '\0';
  }

  d.outdoorIp[0] = '\0';
  if (doc["ip"].is<const char*>()) {
    strncpy(d.outdoorIp, doc["ip"] | "", sizeof(d.outdoorIp) - 1);
    d.outdoorIp[sizeof(d.outdoorIp) - 1] = '\0';
  }

  d.outdoorFirmware[0] = '\0';
  if (doc["firmware"].is<const char*>()) {
    strncpy(d.outdoorFirmware, doc["firmware"] | "", sizeof(d.outdoorFirmware) - 1);
    d.outdoorFirmware[sizeof(d.outdoorFirmware) - 1] = '\0';
  }

  d.mqttHasData = true;
  d.lastMqttMs = millis();

  Serial.print(F("[MQTT] data T="));
  if (weatherValueIsValid(d.temperature)) {
    Serial.print(d.temperature, 1);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" H="));
  if (weatherValueIsValid(d.humidity)) {
    Serial.print(d.humidity, 0);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" P="));
  if (weatherValueIsValid(d.pressure)) {
    Serial.print(d.pressure, 1);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" valid="));
  Serial.print(weatherValueIsValid(d.temperature) ? 1 : 0);
  Serial.print(weatherValueIsValid(d.humidity) ? 1 : 0);
  Serial.print(weatherValueIsValid(d.pressure) ? 1 : 0);
  Serial.print(F(" fresh=1 ext="));
  Serial.println(d.extDesc[0] != '\0' ? d.extDesc : "-");
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  (void)topic;
  // PubSubClient no garantiza payload; copiar a buffer NUL-terminated.
  if (length >= 512) {
    Serial.println(F("[MQTT] payload too large"));
    return;
  }
  char buf[512];
  memcpy(buf, payload, length);
  buf[length] = '\0';
  applyPayload(buf, length);
}

static bool tryConnect() {
  if (millis() < nextRetryMs) {
    return false;
  }

  Serial.print(F("[MQTT] connecting "));
  Serial.print(MQTT_HOST);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);

  const bool ok = mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD);
  if (ok) {
    Serial.println(F("[MQTT] connected"));
    if (mqttClient.subscribe(MQTT_TOPIC)) {
      Serial.print(F("[MQTT] subscribed "));
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.println(F("[MQTT] subscribe fail"));
    }
    backoffMs = 1000;
    nextRetryMs = 0;
    loggedDisconnect = false;
    return true;
  }

  Serial.println(F("[MQTT] fail"));
  nextRetryMs = millis() + backoffMs;
  if (backoffMs < 60000UL) {
    backoffMs *= 2;
    if (backoffMs > 60000UL) {
      backoffMs = 60000UL;
    }
  }
  return false;
}

void MqttSubscriber::begin() {
  const uint32_t chipTail = static_cast<uint32_t>(ESP.getEfuseMac() & 0xFFFFFFULL);
  snprintf(clientId, sizeof(clientId), "%s-%06x", HOSTNAME,
           static_cast<unsigned>(chipTail));

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(30);
  mqttClient.setCallback(onMqttMessage);

  Serial.print(F("[MQTT] broker "));
  Serial.print(MQTT_HOST);
  Serial.print(F(" topic="));
  Serial.print(MQTT_TOPIC);
  Serial.print(F(" clientId="));
  Serial.println(clientId);
}

void MqttSubscriber::update() {
  IndoorData& data = IndoorData::instance();

  if (WiFi.status() != WL_CONNECTED) {
    if (mqttClient.connected()) {
      mqttClient.disconnect();
    }
    data.mqttConnected = false;
    nextRetryMs = 0;
    backoffMs = 1000;
    if (!loggedDisconnect) {
      Serial.println(F("[MQTT] wait wifi"));
      loggedDisconnect = true;
    }
    return;
  }

  if (!mqttClient.connected()) {
    data.mqttConnected = false;
    if (!tryConnect()) {
      yield();
      return;
    }
  }

  data.mqttConnected = true;
  mqttClient.loop();
  yield();
}
