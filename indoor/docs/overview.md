# Indoor — overview

Firmware **0.3.9**. El nodo interior **no** mide sensores de clima. Consume el JSON que publica **outdoor** por MQTT y lo muestra cerca del usuario: **UI = LCD ST7789** (vistas rotativas, incluida Sistema) + reloj MAX7219. OLED removido (no se usa en ninguna placa).

## Flujo

```
outdoor → demo.tbmq.io (topic WeatherStation) → indoor
                                                      ├─ MAX7219  hora NTP (UTC-3)
                                                      └─ LCD      5 vistas ~7 s (temp / hum / presión / OWM / sistema)
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
| LCD (UI) | Adafruit ST7789 + GFX (HSPI MODE3, SCK=23 MOSI=13 DC=4 RST=16 CS=-1) |
| OLED | Removido; no hay hardware ni código |
| LoRa | No usado; RST=GPIO14 **LOW** (no forzar CS=18 HIGH) |

## LCD (0.3.9)

Cinco vistas a pantalla completa, rotación cada `LCD_VIEW_ROTATE_MS` (7000 ms):

1. Temperatura (°C)
2. Humedad (%) + barra
3. Presión (hPa)
4. Condición OWM (`ext_desc` / `ext_temp` / `ext_humidity`) o, si no hay OWM, estado WiFi/MQTT
5. **Sistema**: uptime (`HH:MM:SS` o `Nd HH:MM:SS` si hay días) y free heap (KB + bytes). Si el heap baja de 48 KB (`HEAP_WARN_BYTES`), acento rojo y etiqueta `LOW`.

Serial periódico (~20 s): `[SYS] uptime=… heap=…` (añade `LOW` bajo el mismo umbral).

**Heap / estabilidad (rangos, no una medición única):** en ESP32 Arduino el free heap al boot suele ser ~200–280 KB y baja con WiFi/MQTT. Un valor estable por encima de ~100 KB y un uptime que crece indican que el nodo no está en crash loop. Preocupa una caída continua, heap &lt; 48 KB, o `Reset: 4` (`ESP_RST_PANIC`) al boot. Detalle en [`../README.md`](../README.md#estabilidad--heap).

Cabecera: etiqueta Indoor / “Sin MQTT” y reloj `HH:MM` si NTP está sincronizado. Indicadores de vista en el borde inferior.

## Relación con outdoor

Arquitectura del monorepo: [`../../docs/architecture.md`](../../docs/architecture.md).  
Payload de referencia: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).
