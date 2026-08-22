#include "Max7219Display.h"
#include <string.h>

Max7219Display::~Max7219Display() {
  delete _lc;
  _lc = nullptr;
}

bool Max7219Display::begin(uint8_t dinPin, uint8_t clkPin, uint8_t csPin, bool force) {
  if (_ready && !force) {
    return true;
  }

  delete _lc;
  // LedControl(dataPin, clkPin, csPin, numDevices) — mismo orden que Reloj_Despertador.
  _lc = new LedControl(static_cast<int>(dinPin), static_cast<int>(clkPin),
                       static_cast<int>(csPin), 1);
  if (_lc == nullptr) {
    _ready = false;
    return false;
  }

  // Ctor deja el chip en shutdown; salir a operación normal (como en Reloj_Despertador).
  _lc->shutdown(0, false);
  _lc->setScanLimit(0, 7);
  _lc->setIntensity(0, 2);
  _lc->clearDisplay(0);

  _ready = true;
  return true;
}

void Max7219Display::reassertAfterBusConflict() {
  if (!_ready || _lc == nullptr) {
    return;
  }
  _lc->shutdown(0, false);
  _lc->setScanLimit(0, 7);
}

void Max7219Display::ensureTestOff() {
  // LedControl apaga OP_DISPLAYTEST en el constructor; nada que forzar aquí.
}

void Max7219Display::clear() {
  if (!_ready || _lc == nullptr) {
    return;
  }
  _lc->clearDisplay(0);
}

void Max7219Display::setIntensity(uint8_t level) {
  if (!_ready || _lc == nullptr) {
    return;
  }
  if (level > 15) {
    level = 15;
  }
  _lc->setIntensity(0, level);
}

void Max7219Display::writeDigitOrDash(uint8_t digit, char c) {
  if (_lc == nullptr || digit > 7) {
    return;
  }
  if (c >= '0' && c <= '9') {
    _lc->setDigit(0, digit, static_cast<byte>(c - '0'), false);
  } else if (c == '-') {
    _lc->setChar(0, digit, '-', false);
  } else {
    _lc->setChar(0, digit, ' ', false);
  }
}

void Max7219Display::showDashes() {
  if (!_ready || _lc == nullptr) {
    return;
  }
  for (uint8_t i = 0; i < 8; i++) {
    _lc->setChar(0, i, '-', false);
  }
}

void Max7219Display::showTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  if (!_ready || _lc == nullptr) {
    return;
  }
  if (hours > 23) {
    hours = 0;
  }
  if (minutes > 59) {
    minutes = 0;
  }
  if (seconds > 59) {
    seconds = 0;
  }

  // Digit 7 = MSB (HH), digit 0 = LSB (unidad de segundos) — igual que Reloj_Despertador.
  _lc->setDigit(0, 7, hours / 10, false);
  _lc->setDigit(0, 6, hours % 10, false);
  _lc->setChar(0, 5, '-', false);
  _lc->setDigit(0, 4, minutes / 10, false);
  _lc->setDigit(0, 3, minutes % 10, false);
  _lc->setChar(0, 2, '-', false);
  _lc->setDigit(0, 1, seconds / 10, false);
  _lc->setDigit(0, 0, seconds % 10, false);
}

void Max7219Display::showText(const char* text) {
  if (!_ready || _lc == nullptr) {
    return;
  }
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

  // Texto izquierda → digit 7; derecha → digit 0.
  for (uint8_t i = 0; i < 8; i++) {
    writeDigitOrDash(static_cast<uint8_t>(7 - i), buf[i]);
  }
}
