# Indoor — overview

El nodo interior **no** mide sensores de clima. Consume el JSON que publica **outdoor** por MQTT y lo muestra cerca del usuario en tres displays.

## Flujo

```
outdoor → demo.tbmq.io (topic WeatherStation) → indoor
                                                      ├─ MAX7219  hora (NTP)
                                                      ├─ OLED     diagnóstico
                                                      └─ LCD      UI principal
```

## Stack

| Pieza | Detalle |
|-------|---------|
| Board | NodeMCU v2 (ESP8266) / ideaspark OLED |
| WiFi | WiFiManager (portal AP; sin credenciales en secrets) |
| MQTT | PubSubClient subscribe |
| JSON | ArduinoJson 6 |
| Hora | NTPClient, pool South America, UTC-3 |
| OLED | Adafruit SSD1306 (I2C D6/D5) |
| Reloj | Max7219Display (bit-bang, mismo driver que outdoor antiguo) |
| LCD | Adafruit ST7789 + GFX (soft SPI) |

## Relación con outdoor

Arquitectura del monorepo: [`../../docs/architecture.md`](../../docs/architecture.md).  
Payload de referencia: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).
