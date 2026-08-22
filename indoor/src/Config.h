#pragma once

// ============================================================
// Estación indoor (display) - ESP32 TTGO LoRa32 — Config
// ============================================================

#define FIRMWARE_VERSION "0.3.9"

// Credenciales locales (MQTT; OTA futuro). WiFi: WiFiManager (portal AP).
// Copiá secrets.h.example → secrets.h. No se versiona.
#include "secrets.h"

#ifndef HOSTNAME
#define HOSTNAME "weather-indoor-01"
#endif

// --- WiFiManager ---
// Portal AP = HOSTNAME (ej. weather-indoor-01). http://192.168.4.1
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC  180
#define WIFI_CONNECT_TIMEOUT_SEC        20
#define WIFI_INTERVAL_MS                5000

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
#define LCD_CS_PIN          (-1)  // módulo sin CS
#define LCD_DC_PIN          4
#define LCD_RST_PIN         16
#define LCD_SCK_PIN         23    // VSPI-friendly; no LoRa SCK
#define LCD_MOSI_PIN        13    // no LoRa MOSI
#define LCD_MISO_PIN        (-1)  // no hace falta
#define LCD_WIDTH           240
#define LCD_HEIGHT          240
#define LCD_VIEW_COUNT      5
#define LCD_VIEW_ROTATE_MS  7000  // rotación de vistas dashboard (~6–8 s)
#define HEAP_WARN_BYTES     (48UL * 1024UL)  // LCD/Serial alerta si free heap < 48 KB
#define SYS_SERIAL_INTERVAL_MS  20000

// --- NTP (hora local Argentina UTC-3; reloj autónomo en MAX7219) ---
#define NTP_SERVER              "3.south-america.pool.ntp.org"
#define NTP_TIME_OFFSET_SEC     (-3 * 3600)
#define NTP_UPDATE_INTERVAL_MS  60000
#define NTP_TASK_INTERVAL_MS    1000

// --- MQTT (host / user / password / topic en secrets.h; mismo broker que outdoor) ---
#if !defined(MQTT_HOST) || !defined(MQTT_USER) || !defined(MQTT_PASSWORD) || !defined(MQTT_TOPIC)
#error "Definí MQTT_HOST, MQTT_USER, MQTT_PASSWORD y MQTT_TOPIC en src/secrets.h (copiá secrets.h.example)"
#endif
#define MQTT_PORT               1883
#define MQTT_TASK_INTERVAL_MS   50
#define MQTT_STALE_MS           30000  // sin mensaje → LCD muestra "-" (ajustable)

// --- Intervalos de tareas (ms) ---
#define SYSTEM_INTERVAL_MS      1000
#define CLOCK_INTERVAL_MS       1000
#define LCD_INTERVAL_MS         2000

#define WEATHER_VALUE_INVALID   (-9999.0f)

static inline bool weatherValueIsValid(float v) {
  return v > (WEATHER_VALUE_INVALID + 1.0f);
}
