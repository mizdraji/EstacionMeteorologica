#pragma once

#include <Arduino.h>

// Consulta periódica a OpenWeatherMap (current weather) desde el ESP.
// La API key no se expone al browser.
class ExternalWeather {
public:
  static void begin();
  static void update();
};
