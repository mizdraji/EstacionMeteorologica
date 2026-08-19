#include <Arduino.h>
#include "Config.h"

#ifdef INDOOR_MINIMAL

static void holdBootSafePins() {
  pinMode(MAX7219_DIN_PIN, OUTPUT);
  pinMode(MAX7219_CLK_PIN, OUTPUT);
  digitalWrite(MAX7219_DIN_PIN, LOW);   // GPIO15
  digitalWrite(MAX7219_CLK_PIN, HIGH);  // GPIO2
  pinMode(LCD_RST_PIN, INPUT);          // GPIO0: no forzar LOW
}

void setup() {
  holdBootSafePins();
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("Indoor boot... (MINIMAL)"));
  Serial.println(F("Si ves esto a 115200, el MCU arranca. Flash/strapping OK."));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset reason: "));
  Serial.println(ESP.getResetReason());
  Serial.print(F("Flash chip ID/size: 0x"));
  Serial.print(ESP.getFlashChipId(), HEX);
  Serial.print(F(" / "));
  Serial.println(ESP.getFlashChipRealSize());
}

void loop() {
  static unsigned long n = 0;
  Serial.print(F("[alive] "));
  Serial.print(++n);
  Serial.print(F(" heap="));
  Serial.println(ESP.getFreeHeap());
  delay(1000);
  yield();
}

#else

#include "core/TaskManager.h"

static void holdBootSafePins() {
  pinMode(MAX7219_DIN_PIN, OUTPUT);
  pinMode(MAX7219_CLK_PIN, OUTPUT);
  digitalWrite(MAX7219_DIN_PIN, LOW);
  digitalWrite(MAX7219_CLK_PIN, HIGH);
  pinMode(LCD_RST_PIN, INPUT);
}

void setup() {
  // Strapping seguro ANTES de Serial/drivers
  holdBootSafePins();

  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("Indoor boot..."));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset: "));
  Serial.println(ESP.getResetReason());

  TaskManager::begin();
}

void loop() {
  TaskManager::run();
}

#endif
