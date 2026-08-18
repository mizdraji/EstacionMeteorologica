#pragma once

// ============================================================
// Estación Meteorológica Local - Configuración centralizada
// ============================================================

#define FIRMWARE_VERSION "1.4.0"

// Credenciales locales (OTA). WiFi se configura con WiFiManager (portal AP).
// Ver secrets.h.example. secrets.h no se versiona.
#include "secrets.h"

#define HOSTNAME        "weather-station-01"
#define OTA_HOSTNAME    HOSTNAME

// --- WiFiManager (portal de configuración) ---
// Timeout del portal AP si no hay credenciales o falla la conexión.
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC  180
// Timeout al intentar conectar con credenciales guardadas antes de abrir portal.
#define WIFI_CONNECT_TIMEOUT_SEC        20


// --- I2C BMP180 (tutorial WeatherInstrument: D2/D1) ---
#define BMP180_SDA_PIN  4   // D2
#define BMP180_SCL_PIN  5   // D1

// --- OLED integrado (tutorial WeatherStation: D6/D5) ---
#define OLED_SDA_PIN    12  // D6
#define OLED_SCL_PIN    14  // D5
#define OLED_I2C_ADDRESS 0x3C

// --- AHT10 (mismo bus I2C que BMP180: D2/D1) ---
#define AHT10_I2C_ADDR  0x38

// --- MAX7219 8 digitos (DIN / CLK / CS) ---
#define MAX7219_DIN_PIN  15  // D8
#define MAX7219_CLK_PIN  2   // D4
#define MAX7219_CS_PIN   16  // D0
#define MAX7219_INTENSITY 2  // 0..15

// --- NTP (hora local Argentina UTC-3) ---
#define NTP_SERVER           "3.south-america.pool.ntp.org"
#define NTP_TIME_OFFSET_SEC  (-3 * 3600)
#define NTP_UPDATE_INTERVAL_MS 60000
#define NTP_TASK_INTERVAL_MS 1000

// Presión de referencia al nivel del mar (hPa) para altitud estimada.
// Ajustar según ubicación real; la altitud NO es precisa sin calibrar esto.
#define SEA_LEVEL_PRESSURE_HPA  1013.25f

// --- Intervalos de tareas (ms) ---
#define AHT_INTERVAL_MS         2000
#define BMP180_INTERVAL_MS      1000
#define DISPLAY_INTERVAL_MS     100
#define WIFI_INTERVAL_MS        5000
#define WEB_INTERVAL_MS         50
#define OTA_INTERVAL_MS         30
#define SYSTEM_INTERVAL_MS      1000
#define SERIAL_INTERVAL_MS      5000

// --- Historial web (ring buffer en RAM) ---
// HISTORY_CAPACITY * HISTORY_INTERVAL_MS ~= ventana en minutos
#define HISTORY_INTERVAL_MS     30000
#define HISTORY_CAPACITY        60

// --- Web ---
#define WEB_SERVER_PORT         80

// Valor sentinela para lecturas inválidas (no confundir con 0 real)
#define WEATHER_VALUE_INVALID   (-9999.0f)
