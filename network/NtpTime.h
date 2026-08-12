#pragma once

#include <Arduino.h>

class NtpTime {
public:
  static void begin();
  static void update();
  static bool isSynced();
  static uint8_t hours();
  static uint8_t minutes();
  static uint8_t seconds();
};
