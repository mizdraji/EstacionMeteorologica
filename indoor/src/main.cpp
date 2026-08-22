#include <Arduino.h>
#include "Config.h"
#include <esp_system.h>

#ifdef INDOOR_MINIMAL

static void holdIdlePins() {
  // LoRa en reset (RST LOW). No tocar GPIO18/5: son CLK/CS del MAX.
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);
  pinMode(MAX7219_CS_PIN, OUTPUT);
  digitalWrite(MAX7219_CS_PIN, HIGH);
}

void setup() {
  holdIdlePins();
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("Indoor boot... (MINIMAL)"));
  Serial.println(F("Si ves esto a 115200, el MCU arranca. ESP32 TTGO OK."));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset reason: "));
  Serial.println((int)esp_reset_reason());
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  enableLoopWDT();
}

void loop() {
  static unsigned long n = 0;
  Serial.print(F("[alive] "));
  Serial.print(++n);
  Serial.print(F(" heap="));
  Serial.println(ESP.getFreeHeap());
  delay(1000);
}

#else

#include "core/TaskManager.h"

static void holdIdlePins() {
  // LoRa en reset (RST LOW). No tocar GPIO18/5: son CLK/CS del MAX.
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);
  pinMode(MAX7219_CS_PIN, OUTPUT);
  digitalWrite(MAX7219_CS_PIN, HIGH);
}

void setup() {
  holdIdlePins();

  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("Indoor boot... (ESP32 TTGO LoRa32)"));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset: "));
  Serial.println((int)esp_reset_reason());

  enableLoopWDT();
  TaskManager::begin();
}

void loop() {
  TaskManager::run();
}

#endif
