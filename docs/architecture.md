# Arquitectura del monorepo

## Dispositivos

| Nodo | Ubicación | Función |
|------|-----------|---------|
| **outdoor** | Exterior | Lee sensores (AHT10, BMP180), sirve UI web en LittleFS, publica telemetría MQTT |
| **indoor** | Interior | Suscribe MQTT y visualiza en MAX7219 (hora), OLED (diagnóstico) y LCD ST7789 240×240 |

## Flujo MQTT

1. **outdoor** se conecta al WiFi (WiFiManager) y al broker MQTT.
2. Cada ~`MQTT_INTERVAL_MS` publica un JSON en el topic `WeatherStation`.
3. **indoor** se suscribe al mismo topic, parsea el JSON (ArduinoJson) y actualiza las tres pantallas.
4. La **hora** del MAX7219 indoor viene de **NTP** local (UTC-3), no del payload MQTT.

Topic, host y payload: [`outdoor/docs/mqtt.md`](../outdoor/docs/mqtt.md) · [`indoor/docs/mqtt.md`](../indoor/docs/mqtt.md).

```
outdoor ──publish──► demo.tbmq.io / WeatherStation ──subscribe──► indoor
                                                                    ├ MAX7219
                                                                    ├ OLED
                                                                    └ LCD
```

## Separación de proyectos

Cada carpeta (`outdoor/`, `indoor/`) es un proyecto PlatformIO independiente. Compilar y flashear siempre desde la carpeta del dispositivo correspondiente.
