#pragma once

#include <Arduino.h>
#include <LedControl.h>

/**
 * MAX7219 8 dígitos — wrapper de LedControl (misma lib que ESP8266 / Reloj_Despertador).
 * Constructor LedControl: (data/DIN, clk, cs, numDevices).
 * Convención dígitos: 0 = derecha, 7 = izquierda.
 */
class Max7219Display {
public:
  Max7219Display() = default;
  ~Max7219Display();

  /** Init una sola vez. Re-entradas sin force=false son no-op. */
  bool begin(uint8_t dinPin, uint8_t clkPin, uint8_t csPin, bool force = false);

  /** Re-assert shutdown OFF / scan tras posible conflicto de bus. */
  void reassertAfterBusConflict();

  void clear();
  void setIntensity(uint8_t level);
  void showTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
  void showDashes();
  void showText(const char* text);

  /** LedControl apaga displayTest en el ctor; no-op seguro. */
  void ensureTestOff();

  bool isReady() const { return _ready; }

private:
  LedControl* _lc = nullptr;
  bool _ready = false;

  void writeDigitOrDash(uint8_t digit, char c);
};
