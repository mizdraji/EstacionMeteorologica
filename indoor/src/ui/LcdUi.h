#pragma once

#include "../core/IndoorData.h"

class LcdUi {
public:
  static bool begin();
  static void showBoot();
  static void render(const IndoorData& data);

  /** Reclama HSPI (23/13) antes de dibujar; seguro tras bitbang MAX. */
  static void claimBus();
  /** Fuerza redibujo completo en el próximo render. */
  static void invalidate();

private:
  static bool _ready;
  static unsigned long _lastUpdate;
  static unsigned long _lastViewMs;
  static bool _needsFullRedraw;
  static uint8_t _view;
  static float _lastTemp;
  static float _lastHum;
  static float _lastPress;
  static bool _lastFresh;
  static bool _lastWifi;
  static bool _lastMqtt;
  static char _lastDesc[INDOOR_EXT_DESC_MAX];
  static uint8_t _lastMinute;
  static uint8_t _logSkipCount;

  static void wipeHorizontal(uint16_t color);
  static void drawChrome(const IndoorData& data);
  static void drawView(const IndoorData& data);
  static void drawTempView(const IndoorData& data);
  static void drawHumView(const IndoorData& data);
  static void drawPressView(const IndoorData& data);
  static void drawOwmView(const IndoorData& data);
  static void logRenderDecision(const char* why, const IndoorData& data);
};
