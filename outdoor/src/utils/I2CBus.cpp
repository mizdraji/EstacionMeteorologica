#include "I2CBus.h"

void I2CBus::beginBMP() {
  Wire.begin(BMP180_SDA_PIN, BMP180_SCL_PIN);
  Wire.setClock(100000);
}
