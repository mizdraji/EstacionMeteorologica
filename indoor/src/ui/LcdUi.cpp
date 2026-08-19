#include "LcdUi.h"
#include "../Config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <string.h>

// Soft SPI: pines libres (no chocan con OLED I2C ni MAX7219).
static Adafruit_ST7789 tft = Adafruit_ST7789(LCD_CS_PIN, LCD_DC_PIN, LCD_MOSI_PIN,
                                             LCD_SCK_PIN, LCD_RST_PIN);

bool LcdUi::_ready = false;
unsigned long LcdUi::_lastUpdate = 0;
bool LcdUi::_needsFullRedraw = true;
float LcdUi::_lastTemp = WEATHER_VALUE_INVALID;
float LcdUi::_lastHum = WEATHER_VALUE_INVALID;
float LcdUi::_lastPress = WEATHER_VALUE_INVALID;
bool LcdUi::_lastFresh = false;
char LcdUi::_lastDesc[INDOOR_EXT_DESC_MAX] = {0};

// Paleta legible (sin púrpura / dark-purple defaults)
static const uint16_t COL_BG = 0x18C3;      // azul-gris oscuro
static const uint16_t COL_PANEL = 0x2124;   // panel
static const uint16_t COL_TEMP = 0xFD20;    // naranja
static const uint16_t COL_HUM = 0x07FF;     // cian
static const uint16_t COL_PRESS = 0xAFE5;   // verde suave
static const uint16_t COL_TEXT = 0xEF7D;    // blanco-gris
static const uint16_t COL_MUTED = 0x8410;   // gris
static const uint16_t COL_WARN = 0xF800;    // rojo stale

bool LcdUi::begin() {
  // GPIO0 (D3) = RST: NO forzar LOW al inicio (strapping / flash mode).
  // Adafruit_ST7789 hace soft/hard reset por software en init() — eso es OK
  // porque el chip ya arrancó. Dejar pin en INPUT (pull-up del módulo) antes.
  pinMode(LCD_RST_PIN, INPUT);
  delay(2);

  // Con CS=-1 el módulo debe tener CS a GND.
  tft.init(LCD_WIDTH, LCD_HEIGHT);
  tft.setRotation(0);
  tft.fillScreen(COL_BG);
  _ready = true;
  _needsFullRedraw = true;
  Serial.println(F("[LCD] ST7789 OK"));
  return true;
}

void LcdUi::showBoot() {
  if (!_ready) {
    return;
  }
  tft.fillScreen(COL_BG);
  tft.setTextWrap(false);
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(28, 90);
  tft.print(F("Indoor"));
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(40, 120);
  tft.print(F("MQTT display"));
  tft.setCursor(50, 140);
  tft.print(F("v"));
  tft.print(FIRMWARE_VERSION);
}

void LcdUi::drawBackground() {
  tft.fillScreen(COL_BG);
  tft.fillRoundRect(8, 8, 224, 56, 8, COL_PANEL);
  tft.fillRoundRect(8, 72, 224, 48, 8, COL_PANEL);
  tft.fillRoundRect(8, 128, 224, 48, 8, COL_PANEL);
  tft.fillRoundRect(8, 184, 224, 48, 8, COL_PANEL);

  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(18, 14);
  tft.print(F("TEMPERATURA"));
  tft.setCursor(18, 78);
  tft.print(F("HUMEDAD"));
  tft.setCursor(18, 134);
  tft.print(F("PRESION"));
  tft.setCursor(18, 190);
  tft.print(F("CONDICION OWM"));
}

void LcdUi::drawValues(const IndoorData& data) {
  char buf[24];

  // Temperatura grande
  tft.fillRoundRect(12, 28, 216, 32, 4, COL_PANEL);
  tft.setTextSize(3);
  tft.setTextColor(data.dataIsFresh() ? COL_TEMP : COL_WARN);
  tft.setCursor(18, 30);
  if (weatherValueIsValid(data.temperature)) {
    snprintf(buf, sizeof(buf), "%.1f C", data.temperature);
  } else {
    snprintf(buf, sizeof(buf), "--.- C");
  }
  tft.print(buf);

  // Humedad
  tft.fillRoundRect(12, 90, 216, 24, 4, COL_PANEL);
  tft.setTextSize(2);
  tft.setTextColor(COL_HUM);
  tft.setCursor(18, 94);
  if (weatherValueIsValid(data.humidity)) {
    snprintf(buf, sizeof(buf), "%.0f %% RH", data.humidity);
  } else {
    snprintf(buf, sizeof(buf), "-- %% RH");
  }
  tft.print(buf);

  // Presión
  tft.fillRoundRect(12, 146, 216, 24, 4, COL_PANEL);
  tft.setTextColor(COL_PRESS);
  tft.setCursor(18, 150);
  if (weatherValueIsValid(data.pressure)) {
    snprintf(buf, sizeof(buf), "%.1f hPa", data.pressure);
  } else {
    snprintf(buf, sizeof(buf), "--.- hPa");
  }
  tft.print(buf);

  // Condición OWM / estado
  tft.fillRoundRect(12, 202, 216, 24, 4, COL_PANEL);
  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(18, 210);
  if (data.extDesc[0] != '\0' && data.extOK) {
    tft.print(data.extDesc);
    if (weatherValueIsValid(data.extTemp)) {
      snprintf(buf, sizeof(buf), "  %.0fC", data.extTemp);
      tft.print(buf);
    }
  } else if (!data.mqttConnected) {
    tft.setTextColor(COL_WARN);
    tft.print(F("Sin MQTT"));
  } else if (!data.dataIsFresh()) {
    tft.setTextColor(COL_WARN);
    tft.print(F("Datos antiguos"));
  } else {
    tft.setTextColor(COL_MUTED);
    tft.print(F("Sin OWM"));
  }
}

void LcdUi::render(const IndoorData& data) {
  if (!_ready) {
    return;
  }

  const bool changed =
      _needsFullRedraw ||
      data.dataIsFresh() != _lastFresh ||
      data.temperature != _lastTemp ||
      data.humidity != _lastHum ||
      data.pressure != _lastPress ||
      strncmp(data.extDesc, _lastDesc, INDOOR_EXT_DESC_MAX) != 0;

  if (!changed) {
    return;
  }
  // Throttle SPI redraws even when values change rapidly
  if (!_needsFullRedraw && (millis() - _lastUpdate) < LCD_INTERVAL_MS) {
    return;
  }

  if (_needsFullRedraw) {
    drawBackground();
    _needsFullRedraw = false;
  }

  drawValues(data);

  _lastTemp = data.temperature;
  _lastHum = data.humidity;
  _lastPress = data.pressure;
  _lastFresh = data.dataIsFresh();
  strncpy(_lastDesc, data.extDesc, INDOOR_EXT_DESC_MAX - 1);
  _lastDesc[INDOOR_EXT_DESC_MAX - 1] = '\0';
  _lastUpdate = millis();
}
