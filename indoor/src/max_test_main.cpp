/**
 * Diagnóstico MAX7219 8 dígitos — Serial + LedControl (misma lib que ESP8266).
 * Env: indoor_max_test — sin WiFi / MQTT / LCD / TaskScheduler.
 *
 * Cableado TTGO LoRa32 (Config.h) — LedControl(data, clk, cs):
 *   VCC → 3V3 | GND → GND
 *   DIN → GPIO21  (dataPin)
 *   CLK → GPIO18  (clkPin)
 *   CS  → GPIO5   (csPin)
 *
 * Ciclo visual: 0-9 en todos → "12-34-56" → "88-88-88" → clear → dashes.
 */
#include <Arduino.h>
#include <esp_system.h>
#include "Config.h"
#include "display/Max7219Display.h"

static Max7219Display max7219;

static void holdLoRaInReset() {
  // RST LOW = radio idle. No tocar 18/5: son CLK/CS del MAX.
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
}

static void printWiring() {
  Serial.println(F("[MAX-TEST] === Cableado MAX7219 LedControl(data,clk,cs) ==="));
  Serial.println(F("  VCC → 3V3   GND → GND"));
  Serial.print(F("  DIN → GPIO"));
  Serial.println(MAX7219_DIN_PIN);
  Serial.print(F("  CLK → GPIO"));
  Serial.println(MAX7219_CLK_PIN);
  Serial.print(F("  CS  → GPIO"));
  Serial.println(MAX7219_CS_PIN);
  Serial.println(F("[MAX-TEST] LoRa RST=GPIO14 → LOW (radio en reset)"));
  Serial.println(F("[MAX-TEST] LCD no se inicializa (pines 23/13/4/16 libres)"));
}

void setup() {
  holdLoRaInReset();
  pinMode(MAX7219_CS_PIN, OUTPUT);
  digitalWrite(MAX7219_CS_PIN, HIGH);

  Serial.begin(115200);
  delay(400);
  Serial.flush();
  Serial.println();
  Serial.println(F("Indoor MAX7219 test (LedControl)..."));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset: "));
  Serial.println((int)esp_reset_reason());
  printWiring();

  Serial.println(F("[MAX-TEST] begin LedControl..."));
  if (!max7219.begin(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_CS_PIN, true)) {
    Serial.println(F("[MAX-TEST] begin FAIL"));
    return;
  }
  max7219.setIntensity(MAX7219_INTENSITY);
  max7219.ensureTestOff();
  Serial.println(F("[MAX-TEST] OK — ciclo 0-9 / 12-34-56 / 88-88-88 / clear / dashes"));
}

void loop() {
  if (!max7219.isReady()) {
    delay(1000);
    return;
  }

  // 0..9 en los 8 dígitos
  for (uint8_t d = 0; d <= 9; d++) {
    char buf[9];
    for (uint8_t i = 0; i < 8; i++) {
      buf[i] = static_cast<char>('0' + d);
    }
    buf[8] = '\0';
    Serial.print(F("[MAX-TEST] digits "));
    Serial.println(buf);
    max7219.showText(buf);
    delay(600);
  }

  Serial.println(F("[MAX-TEST] 12-34-56"));
  max7219.showText("12-34-56");
  delay(1200);

  Serial.println(F("[MAX-TEST] 88-88-88"));
  max7219.showText("88-88-88");
  delay(1200);

  Serial.println(F("[MAX-TEST] clear"));
  max7219.clear();
  delay(800);

  Serial.println(F("[MAX-TEST] dashes --------"));
  max7219.showDashes();
  delay(1200);
}
