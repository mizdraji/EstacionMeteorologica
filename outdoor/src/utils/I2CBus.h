#pragma once

#include <Wire.h>
#include "../Config.h"

class I2CBus {
public:
  // Bus I2C compartido AHT10 + BMP180 (D2/D1).
  static void beginBMP();
};
