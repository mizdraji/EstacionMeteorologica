#pragma once

#include "../core/IndoorData.h"

class LcdUi {
public:
  static bool begin();
  static void showBoot();
  static void render(const IndoorData& data);

private:
  static bool _ready;
  static unsigned long _lastUpdate;
  static bool _needsFullRedraw;
  static float _lastTemp;
  static float _lastHum;
  static float _lastPress;
  static bool _lastFresh;
  static char _lastDesc[INDOOR_EXT_DESC_MAX];

  static void drawBackground();
  static void drawValues(const IndoorData& data);
};
