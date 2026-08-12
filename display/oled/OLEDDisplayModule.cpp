#include "OLEDDisplayModule.h"
#include "../../Config.h"
#include "../../utils/I2CBus.h"
#include "SSD1306Wire.h"
#include "WeatherStationFonts.h"

static SSD1306Wire display(OLED_I2C_ADDRESS, OLED_SDA_PIN, OLED_SCL_PIN);
bool OLEDDisplayModule::_ready = false;
unsigned long OLEDDisplayModule::_lastUpdate = 0;

bool OLEDDisplayModule::begin() {
  I2CBus::beginOLED();
  _ready = display.init();
  if (_ready) {
    display.clear();
    display.display();
  }
  return _ready;
}

void OLEDDisplayModule::showBootMessage(const char* line1, const char* line2) {
  if (!_ready) {
    return;
  }
  I2CBus::beginOLED();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, line1);
  display.drawString(64, 30, line2);
  display.display();
}

void OLEDDisplayModule::showOta(uint8_t progress) {
  if (!_ready) {
    return;
  }
  I2CBus::beginOLED();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "OTA Update");
  display.drawProgressBar(10, 28, 108, 10, progress);
  display.display();
}

void OLEDDisplayModule::showDiagnostics(const WeatherData& data) {
  if (!_ready) {
    return;
  }

  if (data.otaInProgress) {
    return;
  }

  if (millis() - _lastUpdate < 1000) {
    return;
  }
  _lastUpdate = millis();

  I2CBus::beginOLED();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  char line[32];
  display.drawString(0, 0, String("FW ") + data.firmwareVersion);

  snprintf(line, sizeof(line), "BMP:%s DHT:%s", data.bmp180OK ? "OK" : "ERR", data.dht11OK ? "OK" : "ERR");
  display.drawString(0, 12, line);

  if (data.wifiConnected) {
    snprintf(line, sizeof(line), "WiFi %d dBm", data.wifiRSSI);
    display.drawString(0, 24, line);
    display.drawString(0, 36, data.ipAddress.toString());
  } else {
    display.drawString(0, 24, "WiFi: offline");
  }

  snprintf(line, sizeof(line), "NTP:%s Up %lus", data.ntpSynced ? "OK" : "--", data.uptime);
  display.drawString(0, 48, line);
  display.display();
}
