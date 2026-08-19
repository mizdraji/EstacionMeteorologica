#pragma once

// ============================================================
// Estación indoor (display) - Configuración centralizada
// ============================================================

#define FIRMWARE_VERSION "0.1.0"

// secrets.h opcional (OTA futuro). WiFi: WiFiManager (portal AP).
// Ver secrets.h.example. secrets.h no se versiona.
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef HOSTNAME
#define HOSTNAME "weather-indoor-01"
#endif

// --- WiFiManager ---
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC  180
#define WIFI_CONNECT_TIMEOUT_SEC        20
#define WIFI_INTERVAL_MS                5000

// --- OLED I2C integrado (ideaspark: SDA=D6, SCL=D5) ---
#define OLED_SDA_PIN      12  // D6
#define OLED_SCL_PIN      14  // D5
#define OLED_I2C_ADDRESS  0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

// --- MAX7219 8 dígitos (hora) ---
#define MAX7219_DIN_PIN   15  // D8
#define MAX7219_CLK_PIN   2   // D4
#define MAX7219_CS_PIN    16  // D0
#define MAX7219_INTENSITY 2   // 0..15

// --- LCD IPS 240×240 ST7789 (SPI bit-bang; pines libres) ---
// CS del módulo: atar a GND (siempre seleccionado). BLK: atar a 3V3.
// RST en GPIO0 (D3): dejar HIGH en boot (reset inactivo). Ver docs/pinout.md.
#define LCD_SCK_PIN       13  // D7
#define LCD_MOSI_PIN      5   // D1
#define LCD_DC_PIN        4   // D2
#define LCD_RST_PIN       0   // D3
#define LCD_CS_PIN        (-1)
#define LCD_WIDTH         240
#define LCD_HEIGHT        240

// --- NTP (hora local Argentina UTC-3; reloj autónomo en MAX7219) ---
#define NTP_SERVER              "3.south-america.pool.ntp.org"
#define NTP_TIME_OFFSET_SEC     (-3 * 3600)
#define NTP_UPDATE_INTERVAL_MS  60000

// --- MQTT (mismo broker demo que outdoor) ---
#define MQTT_HOST               "demo.tbmq.io"
#define MQTT_PORT               1883
#define MQTT_USER               "demo"
#define MQTT_PASSWORD           ""
#define MQTT_TOPIC              "WeatherStation"
#define MQTT_TASK_INTERVAL_MS   50
#define MQTT_STALE_MS           90000  // sin mensaje → UI “stale”

// --- UI refresh ---
#define DISPLAY_INTERVAL_MS     250
#define OLED_INTERVAL_MS        1000
#define LCD_INTERVAL_MS         2000

#define WEATHER_VALUE_INVALID   (-9999.0f)

static inline bool weatherValueIsValid(float v) {
  return v > (WEATHER_VALUE_INVALID + 1.0f);
}
