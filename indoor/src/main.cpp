#include <Arduino.h>
#include "Config.h"

#ifdef INDOOR_MINIMAL

static void holdBootSafePins() {
  pinMode(MAX7219_DIN_PIN, OUTPUT);
  pinMode(MAX7219_CLK_PIN, OUTPUT);
  digitalWrite(MAX7219_DIN_PIN, LOW);
  digitalWrite(MAX7219_CLK_PIN, HIGH);
  pinMode(LCD_RST_PIN, INPUT);
}

void setup() {
  holdBootSafePins();
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("Indoor boot... (MINIMAL)"));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset: "));
  Serial.println(ESP.getResetReason());
}

void loop() {
  static unsigned long n = 0;
  Serial.print(F("[alive] "));
  Serial.println(++n);
  delay(1000);
  yield();
}

#else

#include "core/TaskManager.h"

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("=== Indoor Weather Display ==="));
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