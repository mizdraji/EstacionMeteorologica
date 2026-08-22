#pragma once

// ============================================================
// Estación indoor (display) - ESP32 TTGO LoRa32 — Config
// ============================================================

#define FIRMWARE_VERSION "0.3.7"

// secrets.h opcional (OTA futuro). WiFi: WiFiManager (portal AP).
// Ver secrets.h.example. secrets.h no se versiona.
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef HOSTNAME
#define HOSTNAME "weather-indoor-01"
#endif

// --- WiFiManager ---
// Portal AP = HOSTNAME (ej. weather-indoor-01). http://192.168.4.1
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC  180
#define WIFI_CONNECT_TIMEOUT_SEC        20
#define WIFI_INTERVAL_MS                5000

// --- OLED (placa sin OLED; código opcional deshabilitado) ---
// Pines históricos TTGO V1 (libres tras remoción): SDA=4, SCL=15, RST=16.
#define OLED_ENABLED      0
#define OLED_SDA_PIN      4
#define OLED_SCL_PIN      15
#define OLED_RST_PIN      16
#define OLED_I2C_ADDRESS  0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

// --- LoRa SX127x en PCB (NO usar en este proyecto) ---
// MAX7219 reutiliza SCK/CS del bus LoRa (probado en hardware). El radio se
// mantiene en reset (RST LOW); no forzar CS=18 HIGH (ese pin es MAX CLK).
#define LORA_SCK_PIN      5
#define LORA_MISO_PIN     19
#define LORA_MOSI_PIN     27
#define LORA_CS_PIN       18
#define LORA_RST_PIN      14
#define LORA_IRQ_PIN      26

// --- MAX7219 8 dígitos (hora) — LedControl(data/DIN, clk, cs) ---
// Pines validados por el usuario con LedControl en este TTGO.
#define MAX7219_DIN_PIN   21  // LedControl dataPin
#define MAX7219_CLK_PIN   18  // LedControl clkPin (= LoRa CS en PCB)
#define MAX7219_CS_PIN    5   // LedControl csPin  (= LoRa SCK en PCB)
#define MAX7219_INTENSITY 2   // 0..15

// --- LCD IPS 240×240 ST7789 / GMT130-V1.0 (HW SPI propio) ---
// Módulo sin CS. VCC/BLK → 3V3.
// Bus dedicado: SCK=23, MOSI=13 (no usa el bus LoRa/MAX).
// DC/RST reutilizan pines liberados del OLED (4 / 16).
#define LCD_CS_PIN          (-1)  // módulo sin CS
#define LCD_DC_PIN          4     // OLED SDA liberado
#define LCD_RST_PIN         16    // OLED RST liberado
#define LCD_SCK_PIN         23    // VSPI-friendly; no LoRa SCK
#define LCD_MOSI_PIN        13    // no LoRa MOSI
#define LCD_MISO_PIN        (-1)  // no hace falta
#define LCD_WIDTH           240
#define LCD_HEIGHT          240
#define LCD_VIEW_COUNT      4
#define LCD_VIEW_ROTATE_MS  7000  // rotación de vistas dashboard (~6–8 s)

// --- NTP (hora local Argentina UTC-3; reloj autónomo en MAX7219) ---
#define NTP_SERVER              "3.south-america.pool.ntp.org"
#define NTP_TIME_OFFSET_SEC     (-3 * 3600)
#define NTP_UPDATE_INTERVAL_MS  60000
#define NTP_TASK_INTERVAL_MS    1000

// --- MQTT (mismo broker demo que outdoor) ---
#define MQTT_HOST               "demo.tbmq.io"
#define MQTT_PORT               1883
#define MQTT_USER               "demo"
#define MQTT_PASSWORD           ""
#define MQTT_TOPIC              "WeatherStation"
#define MQTT_TASK_INTERVAL_MS   50
#define MQTT_STALE_MS           90000  // sin mensaje → UI “stale”

// --- Intervalos de tareas (ms) ---
#define SYSTEM_INTERVAL_MS      1000
#define CLOCK_INTERVAL_MS       1000
#define OLED_INTERVAL_MS        1000
#define LCD_INTERVAL_MS         2000

#define WEATHER_VALUE_INVALID   (-9999.0f)

static inline bool weatherValueIsValid(float v) {
  return v > (WEATHER_VALUE_INVALID + 1.0f);
}
