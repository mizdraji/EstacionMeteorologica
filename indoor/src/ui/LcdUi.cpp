#include "LcdUi.h"
#include "../Config.h"
#include "../network/NtpTime.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <string.h>

// LCD en HSPI dedicado (SCK=23 MOSI=13). NO usar SPI global (VSPI):
// los pines por defecto de VSPI son SCK=18 / SS=5 — mismos que MAX LedControl.
static SPIClass lcdSpi(HSPI);
static Adafruit_ST7789 tft = Adafruit_ST7789(&lcdSpi, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN);

bool LcdUi::_ready = false;
unsigned long LcdUi::_lastUpdate = 0;
unsigned long LcdUi::_lastViewMs = 0;
bool LcdUi::_needsFullRedraw = true;
uint8_t LcdUi::_view = 0;
float LcdUi::_lastTemp = WEATHER_VALUE_INVALID;
float LcdUi::_lastHum = WEATHER_VALUE_INVALID;
float LcdUi::_lastPress = WEATHER_VALUE_INVALID;
bool LcdUi::_lastFresh = false;
bool LcdUi::_lastWifi = false;
bool LcdUi::_lastMqtt = false;
char LcdUi::_lastDesc[INDOOR_EXT_DESC_MAX] = {0};
uint8_t LcdUi::_lastMinute = 255;
uint8_t LcdUi::_logSkipCount = 0;
unsigned long LcdUi::_lastUptime = 0;
uint32_t LcdUi::_lastHeap = 0;

static const uint16_t COL_BG = 0x10A2;
static const uint16_t COL_TEMP = 0xFD20;
static const uint16_t COL_HUM = 0x07FF;
static const uint16_t COL_PRESS = 0xAFE5;
static const uint16_t COL_OWM = 0xFFE0;
static const uint16_t COL_TEXT = 0xEF7D;
static const uint16_t COL_MUTED = 0x8410;
static const uint16_t COL_WARN = 0xF800;
static const uint16_t COL_DOT = 0x4A49;

static void formatLiveValue(char* buf, size_t n, const IndoorData& data, float value,
                            const char* fmt) {
  if (data.dataIsFresh() && weatherValueIsValid(value)) {
    snprintf(buf, n, fmt, value);
  } else {
    snprintf(buf, n, "-");
  }
}

static uint16_t accentForView(uint8_t view) {
  switch (view) {
    case 0:
      return COL_TEMP;
    case 1:
      return COL_HUM;
    case 2:
      return COL_PRESS;
    case 3:
      return COL_OWM;
    default:
      return COL_TEXT;
  }
}

static void formatUptime(unsigned long sec, char* buf, size_t n) {
  const unsigned long d = sec / 86400UL;
  const unsigned long h = (sec % 86400UL) / 3600UL;
  const unsigned long m = (sec % 3600UL) / 60UL;
  const unsigned long s = sec % 60UL;
  if (d > 0) {
    snprintf(buf, n, "%luD %02lu:%02lu:%02lu", d, h, m, s);
  } else {
    snprintf(buf, n, "%02lu:%02lu:%02lu", h, m, s);
  }
}

void LcdUi::claimBus() {
  lcdSpi.begin(LCD_SCK_PIN, LCD_MISO_PIN, LCD_MOSI_PIN, -1);
}

void LcdUi::invalidate() {
  _needsFullRedraw = true;
  _lastUpdate = 0;
}

void LcdUi::logRenderDecision(const char* why, const IndoorData& data) {
  Serial.print(F("[LCD] "));
  Serial.print(why);
  Serial.print(F(" v="));
  Serial.print(_view);
  Serial.print(F(" T="));
  if (weatherValueIsValid(data.temperature)) {
    Serial.print(data.temperature, 1);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" H="));
  if (weatherValueIsValid(data.humidity)) {
    Serial.print(data.humidity, 0);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" P="));
  if (weatherValueIsValid(data.pressure)) {
    Serial.print(data.pressure, 1);
  } else {
    Serial.print(F("inv"));
  }
  Serial.print(F(" fresh="));
  Serial.print(data.dataIsFresh() ? 1 : 0);
  Serial.print(F(" mqtt="));
  Serial.print(data.mqttConnected ? 1 : 0);
  Serial.print(F(" has="));
  Serial.println(data.mqttHasData ? 1 : 0);
}

bool LcdUi::begin() {
  Serial.println(F("[LCD] === init GMT130-V1.0 ST7789 HSPI dedicado ==="));
  Serial.println(F("[LCD] ctor Adafruit_ST7789(&lcdSpi/HSPI, CS, DC, RST)"));
  Serial.print(F("[LCD] CS="));
  Serial.print(LCD_CS_PIN);
  Serial.print(F("  DC=GPIO"));
  Serial.print(LCD_DC_PIN);
  Serial.print(F("  RST=GPIO"));
  Serial.println(LCD_RST_PIN);
  Serial.print(F("[LCD] HSPI SCK=GPIO"));
  Serial.print(LCD_SCK_PIN);
  Serial.print(F("  MOSI=GPIO"));
  Serial.println(LCD_MOSI_PIN);
  Serial.println(F("[LCD] cableado: VCC/BLK→3V3, sin pin CS (CS=-1)"));
  Serial.println(F("[LCD] bus HSPI ≠ MAX bitbang 21/18/5; MODE3"));

  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);

  claimBus();
  tft.init(LCD_WIDTH, LCD_HEIGHT, SPI_MODE3);
  tft.setRotation(0);
  tft.invertDisplay(false);

  tft.fillScreen(ST77XX_RED);
  delay(80);
  tft.fillScreen(ST77XX_GREEN);
  delay(80);
  tft.fillScreen(COL_BG);

  _ready = true;
  _needsFullRedraw = true;
  _view = 0;
  _lastViewMs = millis();
  Serial.println(F("[LCD] ST7789 OK (HSPI CS=-1 MODE3)"));
  return true;
}

void LcdUi::showBoot() {
  if (!_ready) {
    return;
  }
  claimBus();
  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, 8, LCD_HEIGHT, COL_TEMP);
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

void LcdUi::wipeHorizontal(uint16_t color) {
  // Franjas verticales: efecto wipe sin delay() ni buffers.
  const int16_t band = 24;
  for (int16_t x = 0; x < LCD_WIDTH; x += band) {
    tft.fillRect(x, 0, band, LCD_HEIGHT, color);
  }
}

void LcdUi::drawChrome(const IndoorData& data) {
  tft.fillRect(0, 0, LCD_WIDTH, 22, COL_BG);
  tft.fillRect(0, 218, LCD_WIDTH, 22, COL_BG);

  tft.setTextWrap(false);
  tft.setTextSize(1);
  if (!data.dataIsFresh()) {
    tft.setTextColor(COL_WARN);
    tft.setCursor(10, 7);
    tft.print(F("Sin MQTT"));
  } else {
    tft.setTextColor(COL_MUTED);
    tft.setCursor(10, 7);
    tft.print(F("Indoor"));
  }

  if (NtpTime::isSynced()) {
    char clk[9];
    snprintf(clk, sizeof(clk), "%02u:%02u", NtpTime::hours(), NtpTime::minutes());
    tft.setTextColor(COL_TEXT);
    tft.setCursor(190, 7);
    tft.print(clk);
  }

  const int16_t startX = (LCD_WIDTH - (LCD_VIEW_COUNT - 1) * 16) / 2;
  for (uint8_t i = 0; i < LCD_VIEW_COUNT; i++) {
    const int16_t cx = startX + (int16_t)i * 16;
    const uint16_t c = (i == _view) ? accentForView(_view) : COL_DOT;
    tft.fillCircle(cx, 228, i == _view ? 4 : 3, c);
  }
}

void LcdUi::drawTempView(const IndoorData& data) {
  tft.fillRect(0, 0, 8, LCD_HEIGHT, COL_TEMP);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 36);
  tft.print(F("TEMPERATURA"));

  char buf[16];
  tft.setTextSize(5);
  tft.setTextColor(data.dataIsFresh() ? COL_TEMP : COL_WARN);
  tft.setCursor(24, 78);
  formatLiveValue(buf, sizeof(buf), data, data.temperature, "%.1f");
  tft.print(buf);

  tft.setTextSize(3);
  tft.setTextColor(COL_TEMP);
  tft.setCursor(24, 148);
  tft.print(F("C"));
}

void LcdUi::drawHumView(const IndoorData& data) {
  tft.fillRect(0, 0, 8, LCD_HEIGHT, COL_HUM);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 36);
  tft.print(F("HUMEDAD"));

  char buf[16];
  const bool liveHum = data.dataIsFresh() && weatherValueIsValid(data.humidity);
  tft.setTextSize(5);
  tft.setTextColor(data.dataIsFresh() ? COL_HUM : COL_WARN);
  tft.setCursor(24, 78);
  formatLiveValue(buf, sizeof(buf), data, data.humidity, "%.0f");
  tft.print(buf);

  tft.setTextSize(3);
  tft.setCursor(24, 148);
  tft.print(F("%"));

  const int16_t barX = 24;
  const int16_t barY = 188;
  const int16_t barW = 192;
  tft.drawRoundRect(barX, barY, barW, 12, 3, COL_MUTED);
  if (liveHum) {
    float h = data.humidity;
    if (h < 0) {
      h = 0;
    }
    if (h > 100) {
      h = 100;
    }
    const int16_t fill = (int16_t)((barW - 4) * h / 100.0f);
    if (fill > 0) {
      tft.fillRoundRect(barX + 2, barY + 2, fill, 8, 2, COL_HUM);
    }
  }
}

void LcdUi::drawPressView(const IndoorData& data) {
  tft.fillRect(0, 0, 8, LCD_HEIGHT, COL_PRESS);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 36);
  tft.print(F("PRESION"));

  char buf[16];
  tft.setTextSize(4);
  tft.setTextColor(data.dataIsFresh() ? COL_PRESS : COL_WARN);
  tft.setCursor(24, 88);
  formatLiveValue(buf, sizeof(buf), data, data.pressure, "%.1f");
  tft.print(buf);

  tft.setTextSize(2);
  tft.setCursor(24, 150);
  tft.print(F("hPa"));
}

void LcdUi::drawOwmView(const IndoorData& data) {
  tft.fillRect(0, 0, 8, LCD_HEIGHT, COL_OWM);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 36);

  const bool hasOwm = data.extOK && data.extDesc[0] != '\0';
  if (hasOwm) {
    const bool live = data.dataIsFresh();
    tft.print(F("CONDICION OWM"));
    tft.setTextWrap(true);
    tft.setTextSize(2);
    tft.setTextColor(live ? COL_TEXT : COL_WARN);
    tft.setCursor(24, 70);
    tft.print(live ? data.extDesc : "-");
    tft.setTextWrap(false);
    if (live && weatherValueIsValid(data.extTemp)) {
      char buf[20];
      snprintf(buf, sizeof(buf), "Ext %.0f C", data.extTemp);
      tft.setTextSize(2);
      tft.setTextColor(COL_OWM);
      tft.setCursor(24, 150);
      tft.print(buf);
    } else if (!live) {
      tft.setTextSize(2);
      tft.setTextColor(COL_WARN);
      tft.setCursor(24, 150);
      tft.print(F("-"));
    }
    if (live && weatherValueIsValid(data.extHumidity)) {
      char buf[20];
      snprintf(buf, sizeof(buf), "HR %.0f %%", data.extHumidity);
      tft.setTextSize(1);
      tft.setTextColor(COL_MUTED);
      tft.setCursor(24, 186);
      tft.print(buf);
    }
    return;
  }

  tft.print(F("ESTADO"));
  tft.setTextWrap(false);
  tft.setTextSize(2);
  tft.setTextColor(data.wifiConnected ? COL_TEXT : COL_WARN);
  tft.setCursor(24, 72);
  if (data.wifiConnected) {
    tft.print(F("WiFi OK"));
    tft.setTextSize(1);
    tft.setTextColor(COL_MUTED);
    tft.setCursor(24, 104);
    tft.print(F("RSSI "));
    tft.print(data.wifiRSSI);
    tft.print(F(" dBm"));
  } else {
    tft.print(F("WiFi off"));
  }

  tft.setTextSize(2);
  tft.setTextColor(data.mqttConnected && data.dataIsFresh() ? COL_TEXT : COL_WARN);
  tft.setCursor(24, 140);
  if (!data.mqttConnected) {
    tft.print(F("Sin MQTT"));
  } else if (!data.dataIsFresh()) {
    tft.print(F("MQTT stale"));
  } else {
    tft.print(F("MQTT OK"));
  }
}

void LcdUi::drawSysView(const IndoorData& data) {
  const bool heapWarn = data.freeHeap < HEAP_WARN_BYTES;
  tft.fillRect(0, 0, 8, LCD_HEIGHT, heapWarn ? COL_WARN : COL_TEXT);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 36);
  tft.print(F("SISTEMA"));

  char buf[20];
  formatUptime(data.uptime, buf, sizeof(buf));
  tft.setTextSize(2);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(24, 72);
  tft.print(F("Uptime"));
  tft.setCursor(24, 100);
  tft.print(buf);

  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 140);
  tft.print(F("Free heap"));
  tft.setTextSize(2);
  tft.setTextColor(heapWarn ? COL_WARN : COL_TEXT);
  tft.setCursor(24, 162);
  snprintf(buf, sizeof(buf), "%lu KB", (unsigned long)(data.freeHeap / 1024UL));
  tft.print(buf);
  tft.setTextSize(1);
  tft.setTextColor(COL_MUTED);
  tft.setCursor(24, 196);
  tft.print(data.freeHeap);
  tft.print(F(" B"));
  if (heapWarn) {
    tft.setTextColor(COL_WARN);
    tft.setCursor(120, 196);
    tft.print(F("LOW"));
  }
}

void LcdUi::drawView(const IndoorData& data) {
  tft.fillScreen(COL_BG);
  tft.setTextWrap(false);
  switch (_view) {
    case 0:
      drawTempView(data);
      break;
    case 1:
      drawHumView(data);
      break;
    case 2:
      drawPressView(data);
      break;
    case 3:
      drawOwmView(data);
      break;
    default:
      drawSysView(data);
      break;
  }
  drawChrome(data);
}

void LcdUi::render(const IndoorData& data) {
  if (!_ready) {
    if (_logSkipCount < 3) {
      Serial.println(F("[LCD] skip: !_ready"));
      _logSkipCount++;
    }
    return;
  }

  const unsigned long now = millis();
  const bool rotate = (now - _lastViewMs) >= LCD_VIEW_ROTATE_MS;
  const uint8_t minute = NtpTime::isSynced() ? NtpTime::minutes() : 255;
  const bool changed =
      _needsFullRedraw || rotate ||
      data.dataIsFresh() != _lastFresh ||
      data.wifiConnected != _lastWifi ||
      data.mqttConnected != _lastMqtt ||
      data.temperature != _lastTemp ||
      data.humidity != _lastHum ||
      data.pressure != _lastPress ||
      minute != _lastMinute ||
      data.uptime != _lastUptime ||
      data.freeHeap != _lastHeap ||
      strncmp(data.extDesc, _lastDesc, INDOOR_EXT_DESC_MAX) != 0;

  if (!changed) {
    return;
  }
  if (!_needsFullRedraw && !rotate && (now - _lastUpdate) < LCD_INTERVAL_MS) {
    return;
  }

  claimBus();

  if (rotate) {
    _view = (uint8_t)((_view + 1) % LCD_VIEW_COUNT);
    _lastViewMs = now;
    logRenderDecision("wipe+view", data);
    wipeHorizontal(accentForView(_view));
    _needsFullRedraw = false;
  } else if (_needsFullRedraw) {
    logRenderDecision("draw FULL", data);
    _needsFullRedraw = false;
  } else {
    logRenderDecision("draw vals", data);
  }

  drawView(data);

  _lastTemp = data.temperature;
  _lastHum = data.humidity;
  _lastPress = data.pressure;
  _lastFresh = data.dataIsFresh();
  _lastWifi = data.wifiConnected;
  _lastMqtt = data.mqttConnected;
  _lastMinute = minute;
  _lastUptime = data.uptime;
  _lastHeap = data.freeHeap;
  strncpy(_lastDesc, data.extDesc, INDOOR_EXT_DESC_MAX - 1);
  _lastDesc[INDOOR_EXT_DESC_MAX - 1] = '\0';
  _lastUpdate = millis();
}
