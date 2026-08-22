#include "OledDisplay.h"
#include "../Config.h"

#if OLED_ENABLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>

// SSD1306 dual-color típico: filas 0–15 amarillas, 16–63 azules.
static const int OLED_TITLE_Y = 0;
static const int OLED_BODY_Y0 = 16;

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RST_PIN);
#endif

bool OledDisplay::_ready = false;
unsigned long OledDisplay::_lastUpdate = 0;

#if OLED_ENABLED
static void drawYellowTitle(const char* title) {
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2);
  const int charW = 12;
  const int len = title ? (int)strlen(title) : 0;
  int x = (OLED_WIDTH - len * charW) / 2;
  if (x < 0) {
    x = 0;
  }
  oled.setCursor(x, OLED_TITLE_Y);
  oled.print(title ? title : "");
}
#endif

bool OledDisplay::begin() {
#if OLED_ENABLED
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
#else
  _ready = false;
  Serial.println(F("[OLED] deshabilitado (placa sin OLED / OLED_ENABLED=0)"));
  return false;
#endif
}

void OledDisplay::showBoot(const char* line1, const char* line2) {
#if OLED_ENABLED
  if (!_ready) {
    return;
  }
  oled.clearDisplay();
  drawYellowTitle(line1 && line1[0] ? line1 : "Weather");

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, OLED_BODY_Y0 + 4);
  oled.println(line2 ? line2 : "");
  oled.setCursor(0, OLED_BODY_Y0 + 20);
  oled.println(F("MQTT outdoor"));
  oled.display();
#else
  (void)line1;
  (void)line2;
#endif
}

void OledDisplay::showStatus(const IndoorData& data) {
#if OLED_ENABLED
  if (!_ready) {
    return;
  }
  if (millis() - _lastUpdate < OLED_INTERVAL_MS) {
    return;
  }
  _lastUpdate = millis();

  oled.clearDisplay();
  drawYellowTitle("Weather");

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  char line[22];
  int y = OLED_BODY_Y0;

  if (weatherValueIsValid(data.temperature)) {
    snprintf(line, sizeof(line), "T  %.1f C", data.temperature);
  } else {
    snprintf(line, sizeof(line), "T  --.- C");
  }
  oled.setCursor(0, y);
  oled.print(line);
  y += 12;

  if (weatherValueIsValid(data.humidity)) {
    snprintf(line, sizeof(line), "H  %.0f %% RH", data.humidity);
  } else {
    snprintf(line, sizeof(line), "H  -- %% RH");
  }
  oled.setCursor(0, y);
  oled.print(line);
  y += 12;

  if (weatherValueIsValid(data.pressure)) {
    snprintf(line, sizeof(line), "P  %.1f hPa", data.pressure);
  } else {
    snprintf(line, sizeof(line), "P  --.- hPa");
  }
  oled.setCursor(0, y);
  oled.print(line);
  y += 12;

  oled.setCursor(0, y);
  if (data.extDesc[0] != '\0' && data.extOK) {
    char desc[21];
    strncpy(desc, data.extDesc, sizeof(desc) - 1);
    desc[sizeof(desc) - 1] = '\0';
    if (weatherValueIsValid(data.extTemp)) {
      snprintf(line, sizeof(line), "%.14s %.0fC", desc, data.extTemp);
    } else {
      snprintf(line, sizeof(line), "%.20s", desc);
    }
    oled.print(line);
  } else if (!data.mqttConnected) {
    oled.print(F("Sin MQTT"));
  } else if (!data.mqttHasData) {
    oled.print(F("Esperando MQTT..."));
  } else if (!data.dataIsFresh()) {
    oled.print(F("Datos antiguos"));
  } else {
    oled.print(F("Sin OWM"));
  }

  oled.display();
#else
  (void)data;
#endif
}
