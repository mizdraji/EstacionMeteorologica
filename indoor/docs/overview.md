# Indoor — overview

Firmware **0.3.7**. El nodo interior **no** mide sensores de clima. Consume el JSON que publica **outdoor** por MQTT y lo muestra cerca del usuario en LCD (vistas rotativas) + reloj MAX7219.

## Flujo

```
outdoor → demo.tbmq.io (topic WeatherStation) → indoor
                                                      ├─ MAX7219  hora NTP (UTC-3)
                                                      └─ LCD      4 vistas ~7 s (temp / hum / presión / OWM)
```

## Stack

| Pieza | Detalle |
|-------|---------|
| Board | ESP32 TTGO LoRa32 V1 (`ttgo-lora32-v1`), env default **`ttgo`** |
| Diagnóstico | `indoor_lcd_test`, `indoor_max_test` (y `indoor_minimal`) |
| WiFi | WiFiManager; AP **`weather-indoor-01`** → http://192.168.4.1 |
| Runtime | TaskScheduler (WiFi, NTP, MQTT, MAX7219, LCD) |
| MQTT | PubSubClient subscribe, topic `WeatherStation` |
| JSON | ArduinoJson 6 |
| Hora | NTPClient, `3.south-america.pool.ntp.org`, UTC-3 |
| Reloj | Max7219Display + LedControl local (DIN=21 CLK=18 CS=5) |
| LCD | Adafruit ST7789 + GFX (HSPI MODE3, SCK=23 MOSI=13 DC=4 RST=16 CS=-1) |
| OLED | Deshabilitado (`OLED_ENABLED=0`) |
| LoRa | No usado; RST=GPIO14 **LOW** (no forzar CS=18 HIGH) |

## LCD (0.3.7)

Cuatro vistas a pantalla completa, rotación cada `LCD_VIEW_ROTATE_MS` (7000 ms):

1. Temperatura (°C)
2. Humedad (%) + barra
3. Presión (hPa)
4. Condición OWM (`ext_desc` / `ext_temp` / `ext_humidity`) o, si no hay OWM, estado WiFi/MQTT

Cabecera: etiqueta Indoor / “Sin MQTT” y reloj `HH:MM` si NTP está sincronizado. Indicadores de vista en el borde inferior. **No** hay uptime ni heap en el LCD.

## Relación con outdoor

Arquitectura del monorepo: [`../../docs/architecture.md`](../../docs/architecture.md).  
Payload de referencia: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).
