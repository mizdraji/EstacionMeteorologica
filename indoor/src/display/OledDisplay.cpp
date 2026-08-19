#include "OledDisplay.h"
#include "../Config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

bool OledDisplay::_ready = false;
unsigned long OledDisplay::_lastUpdate = 0;

bool OledDisplay::begin() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  _ready = oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  if (!_ready) {
    Serial.println(F("[OLED] init fail"));
    return false;
  }
  oled.clearDisplay();
  oled.display();
  Serial.println(F("[OLED] OK"));
  return true;
}

void OledDisplay::showBoot(const char* line1, const char* line2) {
  if (!_ready) {
    return;
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 16);
  oled.println(line1 ? line1 : "");
  oled.setCursor(0, 32);
  oled.println(line2 ? line2 : "");
  oled.display();
}

void OledDisplay::showStatus(const IndoorData& data) {
  if (!_ready) {
    return;
  }
  if (millis() - _lastUpdate < OLED_INTERVAL_MS) {
    return;
  }
  _lastUpdate = millis();

  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);

  char line[22];

  // L0: temp + humidity compactos
  if (weatherValueIsValid(data.temperature) && weatherValueIsValid(data.humidity)) {
    snprintf(line, sizeof(line), "%.1fC  %.0f%%RH", data.temperature, data.humidity);
  } else if (data.mqttHasData) {
    snprintf(line, sizeof(line), "MQTT stale/partial");
  } else {
    snprintf(line, sizeof(line), "Esperando MQTT...");
  }
  oled.setCursor(0, 0);
  oled.print(line);

  // L1: presión
  if (weatherValueIsValid(data.pressure)) {
    snprintf(line, sizeof(line), "P %.1f hPa", data.pressure);
  } else {
    snprintf(line, sizeof(line), "P --");
  }
  oled.setCursor(0, 12);
  oled.print(line);

  // L2: WiFi
  if (data.wifiConnected) {
    snprintf(line, sizeof(line), "WiFi %d dBm", (int)data.wifiRSSI);
  } else {
    snprintf(line, sizeof(line), "WiFi offline");
  }
  oled.setCursor(0, 24);
  oled.print(line);

  // L3: IP
  oled.setCursor(0, 36);
  if (data.wifiConnected) {
    oled.print(data.ipAddress.toString());
  } else {
    oled.print(F("AP: 192.168.4.1"));
  }

  // L4: MQTT + heap
  snprintf(line, sizeof(line), "MQTT:%s H:%u",
           data.mqttConnected ? (data.dataIsFresh() ? "OK" : "STALE") : "--",
           (unsigned)data.freeHeap);
  oled.setCursor(0, 48);
  oled.print(line);

  oled.display();
}
