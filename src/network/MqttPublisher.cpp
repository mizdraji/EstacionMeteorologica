#include "MqttPublisher.h"
#include "../Config.h"
#include "../core/WeatherData.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

static char clientId[40];
static unsigned long lastPublishMs = 0;
static unsigned long nextRetryMs = 0;
static unsigned long backoffMs = 1000;
static bool loggedDisconnect = false;

static void setJsonFloatOrNull(JsonObject obj, const char* key, float value) {
  if (weatherValueIsValid(value)) {
    obj[key] = roundf(value * 10.0f) / 10.0f;
  } else {
    obj[key] = nullptr;
  }
}

static bool tryConnect() {
  if (millis() < nextRetryMs) {
    return false;
  }

  Serial.print(F("[MQTT] connecting "));
  Serial.print(MQTT_HOST);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);

  // Password vacía "" es válida en este broker demo (no pasar nullptr).
  const bool ok = mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD);
  if (ok) {
    Serial.println(F("[MQTT] connected"));
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

static bool publishTelemetry() {
  WeatherData& data = WeatherData::instance();

  StaticJsonDocument<384> doc;
  JsonObject root = doc.to<JsonObject>();

  setJsonFloatOrNull(root, "temp_main", data.temperatureMain);
  setJsonFloatOrNull(root, "temp_aht", data.temperatureAHT);
  setJsonFloatOrNull(root, "temp_bmp", data.temperatureBMP);
  setJsonFloatOrNull(root, "humidity", data.humidity);
  setJsonFloatOrNull(root, "pressure", data.pressure);
  setJsonFloatOrNull(root, "altitude", data.altitude);

  root["bmp180_ok"] = data.bmp180OK;
  root["aht10_ok"] = data.aht10OK;

  setJsonFloatOrNull(root, "ext_temp", data.externalTemperature);
  setJsonFloatOrNull(root, "ext_humidity", data.externalHumidity);
  root["ext_ok"] = data.externalOK;
  if (data.externalDescription[0] != '\0') {
    root["ext_desc"] = data.externalDescription;
  }

  root["ip"] = data.ipAddress.toString();
  root["uptime"] = data.uptime;
  root["firmware"] = FIRMWARE_VERSION;
  root["heap"] = data.freeHeap;
  root["rssi"] = data.wifiRSSI;

  char payload[384];
  const size_t n = serializeJson(doc, payload, sizeof(payload));
  if (n == 0 || n >= sizeof(payload)) {
    Serial.println(F("[MQTT] publish fail (payload)"));
    return false;
  }

  if (mqttClient.publish(MQTT_TOPIC, payload, false)) {
    Serial.println(F("[MQTT] publish"));
    return true;
  }

  Serial.println(F("[MQTT] publish fail"));
  return false;
}

void MqttPublisher::begin() {
  snprintf(clientId, sizeof(clientId), "%s-%06x", HOSTNAME,
           static_cast<unsigned>(ESP.getChipId() & 0xFFFFFF));

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setBufferSize(384);
  mqttClient.setKeepAlive(30);

  Serial.print(F("[MQTT] demo broker "));
  Serial.print(MQTT_HOST);
  Serial.print(F(" topic="));
  Serial.print(MQTT_TOPIC);
  Serial.print(F(" clientId="));
  Serial.println(clientId);
}

void MqttPublisher::update() {
  if (WiFi.status() != WL_CONNECTED) {
    if (mqttClient.connected()) {
      mqttClient.disconnect();
    }
    // Sin WiFi: no spamear reconnect; reset suave del backoff.
    nextRetryMs = 0;
    backoffMs = 1000;
    if (!loggedDisconnect) {
      Serial.println(F("[MQTT] wait wifi"));
      loggedDisconnect = true;
    }
    return;
  }

  if (!mqttClient.connected()) {
    if (!tryConnect()) {
      return;
    }
    // Publicar pronto tras reconnect (sin esperar un ciclo completo).
    lastPublishMs = 0;
  }

  mqttClient.loop();

  const unsigned long now = millis();
  if (lastPublishMs != 0 && (now - lastPublishMs) < MQTT_INTERVAL_MS) {
    return;
  }

  if (publishTelemetry()) {
    lastPublishMs = now;
  } else if (mqttClient.connected()) {
    // Broker ocupado / buffer: reintentar sin desconectar.
    nextRetryMs = now + backoffMs;
  }
}
