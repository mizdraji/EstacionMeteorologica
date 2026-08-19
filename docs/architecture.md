# Arquitectura del monorepo

## Dispositivos

| Nodo | Ubicación | Función |
|------|-----------|---------|
| **outdoor** | Exterior | Lee sensores (AHT10, BMP180), sirve UI web en LittleFS, publica telemetría MQTT |
| **indoor** | Interior | Consume telemetría MQTT y la visualiza (display / UI) — en construcción |

## Flujo MQTT

1. **outdoor** se conecta al WiFi (WiFiManager) y al broker MQTT.
2. Cada ~`MQTT_INTERVAL_MS` publica un JSON en el topic configurado (p. ej. `WeatherStation`).
3. **indoor** (futuro) se suscribe a ese topic, parsea el JSON y actualiza la visualización.

Topic, host y payload de referencia del exterior: [`outdoor/docs/mqtt.md`](../outdoor/docs/mqtt.md).

## Separación de proyectos

Cada carpeta (`outdoor/`, `indoor/`) es un proyecto PlatformIO independiente (`platformio.ini` propio). Compilar y flashear siempre desde la carpeta del dispositivo correspondiente.
