#include "I2CBus.h"

void I2CBus::beginBMP() {
  Wire.begin(BMP180_SDA_PIN, BMP180_SCL_PIN);
  Wire.setClock(100000);
}

void I2CBus::beginOLED() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  Wire.setClock(400000);
}
