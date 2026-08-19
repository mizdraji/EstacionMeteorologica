#include <Arduino.h>
#include "Config.h"
#include "core/IndoorData.h"
#include "network/StationWiFi.h"
#include "network/MqttSubscriber.h"
#include "network/NtpTime.h"
#include "display/Max7219Display.h"
#include "display/OledDisplay.h"
#include "ui/LcdUi.h"

static StationWiFi wifi;
static Max7219Display max7219;
static unsigned long lastDisplayMs = 0;
static unsigned long lastSecond = 0;

static void updateClock() {
  IndoorData& data = IndoorData::instance();
  if (!data.max7219OK) {
    return;
  }
  if (NtpTime::isSynced()) {
    max7219.showTime(NtpTime::hours(), NtpTime::minutes(), NtpTime::seconds());
  } else {
    max7219.showDashes();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Indoor Weather Display ==="));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);

  IndoorData& data = IndoorData::instance();

  // MAX7219 primero (pines boot-safe excepto D8 ya usado como DIN)
  data.max7219OK = max7219.begin(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_CS_PIN);
  if (data.max7219OK) {
    max7219.setIntensity(MAX7219_INTENSITY);
    max7219.showDashes();
    Serial.println(F("[MAX7219] OK"));
  }

  data.oledOK = OledDisplay::begin();
  if (data.oledOK) {
    OledDisplay::showBoot("Indoor", FIRMWARE_VERSION);
  }

  data.lcdOK = LcdUi::begin();
  if (data.lcdOK) {
    LcdUi::showBoot();
  }

  wifi.begin();
  NtpTime::begin();
  MqttSubscriber::begin();

  Serial.println(F("[BOOT] listo"));
}

void loop() {
  IndoorData& data = IndoorData::instance();

  wifi.update();
  NtpTime::update();
  MqttSubscriber::update();

  data.uptime = millis() / 1000UL;
  data.freeHeap = ESP.getFreeHeap();

  const unsigned long now = millis();

  // Reloj MAX7219 cada segundo
  const unsigned long sec = now / 1000UL;
  if (sec != lastSecond) {
    lastSecond = sec;
    updateClock();
  }

  if (now - lastDisplayMs >= DISPLAY_INTERVAL_MS) {
    lastDisplayMs = now;
    OledDisplay::showStatus(data);
    LcdUi::render(data);
  }

  yield();
}
