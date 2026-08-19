# Indoor — overview

El nodo interior **no** mide el clima exterior. Consume lo que publica el outdoor y lo muestra cerca del usuario.

## Objetivos (plan)

1. Conectarse al mismo broker MQTT que usa outdoor.
2. Suscribirse al topic de telemetría (p. ej. `WeatherStation`; ver `outdoor/docs/mqtt.md`).
3. Parsear el JSON (temp, humedad, presión, estado de sensores, etc.).
4. Visualizar en display local (LCD/OLED u otro — por definir) y/o UI auxiliar.

## Fuera de alcance por ahora

- Firmware completo de display
- Elección definitiva de placa (ESP32 vs otra)
- Credenciales / `secrets.h` reales

## Relación con outdoor

```
outdoor → MQTT broker → indoor (esta carpeta)
```

Arquitectura del monorepo: [`../../docs/architecture.md`](../../docs/architecture.md).
