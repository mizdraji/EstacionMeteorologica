#pragma once

#include <Arduino.h>
#include "../Config.h"

struct HistorySample {
  uint32_t epoch;
  float temperature;
  float humidity;
  float pressure;
};

class HistoryBuffer {
public:
  static void begin();
  static void pushFromWeatherData();
  static uint16_t count();
  static uint16_t capacity();
  static uint32_t intervalSec();
  static bool getSample(uint16_t indexFromOldest, HistorySample& out);

private:
  static HistorySample _samples[HISTORY_CAPACITY];
  static uint16_t _head;
  static uint16_t _count;
};
