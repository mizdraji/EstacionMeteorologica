#include "Max7219Display.h"
#include <string.h>

bool Max7219Display::begin(uint8_t dinPin, uint8_t clkPin, uint8_t csPin) {
  _din = dinPin;
  _clk = clkPin;
  _cs = csPin;

  // ESP8266 strapping: GPIO15 (D8) debe quedar LOW, GPIO2 (D4) HIGH
  // antes de cualquier tráfico SPI hacia el MAX7219.
  pinMode(_din, OUTPUT);
  pinMode(_clk, OUTPUT);
  pinMode(_cs, OUTPUT);
  digitalWrite(_din, LOW);   // GPIO15 boot-safe
  digitalWrite(_clk, HIGH);  // GPIO2 boot-safe (no dejar LOW)
  digitalWrite(_cs, HIGH);
  delay(1);

  send(0x0F, 0x00);  // display test off
  send(0x0C, 0x01);  // normal operation
  send(0x0B, 0x07);  // scan all 8 digits
  send(0x09, 0xFF);  // BCD decode for all digits
  setIntensity(2);
  clear();

  // Tras hablar con el chip, restaurar strapping por si hay WDT/reset
  digitalWrite(_din, LOW);
  digitalWrite(_clk, HIGH);
  return true;
}

void Max7219Display::send(uint8_t address, uint8_t data) {
  digitalWrite(_cs, LOW);
  shiftOut(_din, _clk, MSBFIRST, address);
  shiftOut(_din, _clk, MSBFIRST, data);
  digitalWrite(_cs, HIGH);
  // shiftOut deja CLK LOW; GPIO2 debe volver a HIGH (strapping)
  if (_clk == 2) {
    digitalWrite(_clk, HIGH);
  }
}

void Max7219Display::setDigitRaw(uint8_t digit, uint8_t value) {
  if (digit > 7) {
    return;
  }
  send(digit + 1, value);
}

void Max7219Display::clear() {
  for (uint8_t i = 0; i < 8; i++) {
    setDigitRaw(i, 0x0F);
  }
}

void Max7219Display::setIntensity(uint8_t level) {
  if (level > 15) {
    level = 15;
  }
  send(0x0A, level);
}

void Max7219Display::showDashes() {
  setDigitRaw(7, 0x0A);
  setDigitRaw(6, 0x0A);
  setDigitRaw(5, 0x0A);
  setDigitRaw(4, 0x0A);
  setDigitRaw(3, 0x0A);
  setDigitRaw(2, 0x0A);
  setDigitRaw(1, 0x0A);
  setDigitRaw(0, 0x0A);
}

void Max7219Display::showTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  if (hours > 23) {
    hours = 0;
  }
  if (minutes > 59) {
    minutes = 0;
  }
  if (seconds > 59) {
    seconds = 0;
  }

  setDigitRaw(7, hours / 10);
  setDigitRaw(6, hours % 10);
  setDigitRaw(5, 0x0A);
  setDigitRaw(4, minutes / 10);
  setDigitRaw(3, minutes % 10);
  setDigitRaw(2, 0x0A);
  setDigitRaw(1, seconds / 10);
  setDigitRaw(0, seconds % 10);
}

void Max7219Display::showText(const char* text) {
  char buf[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
  if (text != nullptr) {
    size_t len = strlen(text);
    if (len > 8) {
      len = 8;
    }
    for (size_t i = 0; i < len; i++) {
      buf[i] = text[i];
    }
  }

  for (uint8_t i = 0; i < 8; i++) {
    const char c = buf[i];
    uint8_t value = 0x0F;
    if (c >= '0' && c <= '9') {
      value = (uint8_t)(c - '0');
    } else if (c == '-') {
      value = 0x0A;
    }
    setDigitRaw(7 - i, value);
  }
}
