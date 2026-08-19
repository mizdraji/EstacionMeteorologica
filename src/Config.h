#pragma once

// ============================================================
// Estación Meteorológica Local - Configuración centralizada
// ============================================================

#define FIRMWARE_VERSION "1.6.1"

// Credenciales locales (OTA + OpenWeatherMap). WiFi se configura con WiFiManager (portal AP).
// Ver secrets.h.example. secrets.h no se versiona.
#include "secrets.h"

#define HOSTNAME        "weather-station-01"
#define OTA_HOSTNAME    HOSTNAME

// --- WiFiManager (portal de configuración) ---
// Timeout del portal AP si no hay credenciales o falla la conexión.
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC  180
// Timeout al intentar conectar con credenciales guardadas antes de abrir portal.
#define WIFI_CONNECT_TIMEOUT_SEC        20


// --- I2C sensores (NodeMCU: D2=SDA / D1=SCL) ---
// AHT10 + BMP180 comparten el mismo bus.
#define BMP180_SDA_PIN  4   // D2
#define BMP180_SCL_PIN  5   // D1

// --- AHT10 (mismo bus I2C que BMP180: D2/D1) ---
#define AHT10_I2C_ADDR  0x38

// --- NTP (hora local Argentina UTC-3; visible en la web) ---
#define NTP_SERVER           "3.south-america.pool.ntp.org"
#define NTP_TIME_OFFSET_SEC  (-3 * 3600)
#define NTP_UPDATE_INTERVAL_MS 60000
#define NTP_TASK_INTERVAL_MS 1000

// Presión de referencia al nivel del mar (hPa) para altitud estimada.
// Ajustar según ubicación real; la altitud NO es precisa sin calibrar esto.
#define SEA_LEVEL_PRESSURE_HPA  1013.25f

// --- OpenWeatherMap (referencia externa; consulta el ESP, no el browser) ---
// 1 = city id; 0 = lat/lon (recomendado; no hace falta city id)
#define OWM_USE_CITY_ID         0
#define OWM_CITY_ID             3435217  // solo si OWM_USE_CITY_ID = 1
#define OWM_LAT                 (-27.47f)
#define OWM_LON                 (-58.83f)
#define OWM_INTERVAL_MS         600000   // 10 min (free tier / heap)
#define EXTERNAL_DESC_MAX       48
#define OWM_TASK_INTERVAL_MS    5000     // poll interno; fetch real cada OWM_INTERVAL_MS

// --- MQTT (broker demo público ThingsBoard; ver docs/mqtt.md) ---
// Password vacía "" (string vacío, no nullptr). Credenciales de demo, no secretas.
#define MQTT_HOST               "demo.tbmq.io"
#define MQTT_PORT               1883
#define MQTT_USER               "demo"
#define MQTT_PASSWORD           ""
#define MQTT_TOPIC              "WeatherStation"
#define MQTT_INTERVAL_MS        30000   // alineado a HISTORY_INTERVAL_MS
#define MQTT_TASK_INTERVAL_MS   1000    // loop/reconnect no bloqueante

// --- Intervalos de tareas (ms) ---
#define AHT_INTERVAL_MS         2000
#define BMP180_INTERVAL_MS      1000
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
