#pragma once

#include <Arduino.h>

class Max7219Display {
public:
  bool begin(uint8_t dinPin, uint8_t clkPin, uint8_t csPin);
  void clear();
  void setIntensity(uint8_t level);
  void showTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
  void showDashes();
  void showText(const char* text);

private:
  uint8_t _din = 0;
  uint8_t _clk = 0;
  uint8_t _cs = 0;

  void send(uint8_t address, uint8_t data);
  void setDigitRaw(uint8_t digit, uint8_t value);
};
