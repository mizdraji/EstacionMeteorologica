# Arquitectura del monorepo

## Dispositivos

| Nodo | Ubicación | Función |
|------|-----------|---------|
| **outdoor** | Exterior | Lee sensores (AHT10, BMP180), sirve UI web en LittleFS, publica telemetría MQTT |
| **indoor** | Interior | Firmware **0.3.9**. Suscribe MQTT; UI = LCD ST7789 240×240 (5 vistas ~7 s, incluida Sistema) + MAX7219 (hora NTP UTC-3). ESP32 TTGO LoRa32 V1 u otra placa: **siempre sin OLED**. LoRa RST=14 LOW |

## Flujo MQTT

1. **outdoor** se conecta al WiFi (WiFiManager) y al broker MQTT.
2. Cada ~`MQTT_INTERVAL_MS` publica un JSON en el topic definido en `secrets.h`.
3. **indoor** se suscribe al mismo topic, parsea el JSON (ArduinoJson) y actualiza el LCD. Si no llega un mensaje en `MQTT_STALE_MS`, la UI muestra `-` en lugar del último valor.
4. La **hora** del MAX7219 indoor viene de **NTP** local (UTC-3), no del payload MQTT.

Host, usuario, password y topic MQTT van en `src/secrets.h` de cada nodo. Puerto y payload: [`outdoor/docs/mqtt.md`](../outdoor/docs/mqtt.md) · [`indoor/docs/mqtt.md`](../indoor/docs/mqtt.md).

```
outdoor ──publish──► broker MQTT (secrets.h) ──subscribe──► indoor
                                                                    ├ MAX7219
                                                                    └ LCD
```

## Separación de proyectos

Cada carpeta (`outdoor/`, `indoor/`) es un proyecto PlatformIO independiente. Compilar y flashear siempre desde la carpeta del dispositivo correspondiente.
