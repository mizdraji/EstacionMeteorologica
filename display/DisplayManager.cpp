#include "DisplayManager.h"
#include "../Config.h"
#include "oled/OLEDDisplayModule.h"
#include "../network/NtpTime.h"

bool DisplayManager::begin() {
  bool maxOk = _max7219.begin(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_CS_PIN);
  if (maxOk) {
    _max7219.setIntensity(MAX7219_INTENSITY);
    _max7219.showDashes();
  }
  WeatherData::instance().max7219OK = maxOk;

  bool oled = OLEDDisplayModule::begin();
  WeatherData::instance().oledOK = oled;

  return maxOk || oled;
}

void DisplayManager::renderClock(const WeatherData& data) {
  if (data.otaInProgress) {
    char otaText[9];
    snprintf(otaText, sizeof(otaText), "OTA %3u", data.otaProgress);
    if (data.max7219OK) {
      _max7219.showText(otaText);
    }
    OLEDDisplayModule::showOta(data.otaProgress);
    return;
  }

  if (data.max7219OK) {
    if (NtpTime::isSynced()) {
      _max7219.showTime(NtpTime::hours(), NtpTime::minutes(), NtpTime::seconds());
    } else {
      _max7219.showDashes();
    }
  }

  OLEDDisplayModule::showDiagnostics(data);
}

void DisplayManager::update() {
  renderClock(WeatherData::instance());
}
