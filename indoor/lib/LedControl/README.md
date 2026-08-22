# LedControl (vendored)

Misma API que [wayoda/LedControl](https://github.com/wayoda/LedControl) usada en Reloj_Despertador / ESP8266.

Parche local vs registry 1.0.6: `#include <pgmspace.h>` en ESP32/ESP8266 (el upstream solo incluye `avr/pgmspace.h` y no compila en ESP).

Ctor: `LedControl(dataPin, clkPin, csPin, numDevices)`.
