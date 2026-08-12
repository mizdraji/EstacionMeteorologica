#pragma once

#include <Wire.h>
#include "../Config.h"

class I2CBus {
public:
  static void beginBMP();
  static void beginOLED();
};
