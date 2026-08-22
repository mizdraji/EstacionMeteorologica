/**
 * Diagnóstico LCD GMT130-V1.0 (240×240 IPS ST7789, sin CS).
 * Env: indoor_lcd_test — HW SPI ESP32 (bus propio, no LoRa). Sin WiFi/MQTT/MAX.
 *
 * Cableado TTGO LoRa32:
 *   VCC, BLK → 3V3 | GND → GND
 *   DC  → GPIO4
 *   RST → GPIO16
 *   SCK → GPIO23
 *   MOSI→ GPIO13
 *   CS  N/A (-1)
 *
 * Ctor: Adafruit_ST7789(&SPI, CS, DC, RST)
 * Init: SPI.begin(sck, miso, mosi, ss); tft.init(240, 240, SPI_MODE3);
 */
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <esp_system.h>
#include "Config.h"

static Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN);

static void holdLoRaInReset() {
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
}

static void printWiring() {
  Serial.println(F("[LCD-TEST] === Cableado GMT130 (HW SPI ESP32) ==="));
  Serial.println(F("  VCC → 3V3   BLK → 3V3   GND → GND"));
  Serial.println(F("  CS  N/A (modulo sin CS; ctor CS=-1)"));
  Serial.print(F("  DC  → GPIO"));
  Serial.println(LCD_DC_PIN);
  Serial.print(F("  RST → GPIO"));
  Serial.println(LCD_RST_PIN);
  Serial.print(F("  SCK → GPIO"));
  Serial.println(LCD_SCK_PIN);
  Serial.print(F("  MOSI→ GPIO"));
  Serial.println(LCD_MOSI_PIN);
  Serial.println();
  Serial.println(F("[LCD-TEST] Adafruit_ST7789(&SPI, CS, DC, RST)"));
  Serial.print(F("  CS="));
  Serial.print(LCD_CS_PIN);
  Serial.print(F(" DC=GPIO"));
  Serial.print(LCD_DC_PIN);
  Serial.print(F(" RST=GPIO"));
  Serial.println(LCD_RST_PIN);
  Serial.println(F("[LCD-TEST] LoRa RST=GPIO14 → LOW (radio en reset)"));
}

void setup() {
  holdLoRaInReset();
  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);

  Serial.begin(115200);
  delay(400);
  Serial.flush();

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("LCD test — Adafruit ST7789 HW SPI ESP32"));
  Serial.println(F("GMT130 240x240  CS=-1  MODE3  (no bus LoRa)"));
  Serial.println(F("========================================"));
  Serial.print(F("FW "));
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Reset: "));
  Serial.println((int)esp_reset_reason());
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  printWiring();
  Serial.flush();

  Serial.println(F("[LCD-TEST] SPI.begin(sck,-1,mosi,-1) + init MODE3..."));
  Serial.flush();

  SPI.begin(LCD_SCK_PIN, LCD_MISO_PIN, LCD_MOSI_PIN, -1);
  tft.init(LCD_WIDTH, LCD_HEIGHT, SPI_MODE3);
  tft.setRotation(0);
  Serial.println(F("[LCD-TEST] init() OK — mirar colores / HELLO"));

  tft.fillScreen(ST77XX_RED);
  delay(800);
  tft.fillScreen(ST77XX_GREEN);
  delay(800);
  tft.fillScreen(ST77XX_BLUE);
  delay(800);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.print(F("HELLO"));

  Serial.println(F("[LCD-TEST] Rojo/verde/azul + HELLO listo."));
  Serial.println(F("========================================"));
  Serial.flush();
}

void loop() {
  static unsigned long n = 0;
  Serial.print(F("[LCD-TEST alive] "));
  Serial.print(++n);
  Serial.print(F(" heap="));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F(" uptime_ms="));
  Serial.println(millis());
  Serial.flush();
  delay(2000);
}
