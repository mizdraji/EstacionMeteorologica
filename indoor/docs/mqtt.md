# Indoor — MQTT (suscripción)

El indoor se **suscribe** al mismo broker/topic que usa outdoor para publicar.

## Parámetros (`src/Config.h`)

| Define | Valor |
|--------|-------|
| `MQTT_HOST` | `demo.tbmq.io` |
| `MQTT_PORT` | `1883` |
| `MQTT_USER` | `demo` |
| `MQTT_PASSWORD` | `""` |
| `MQTT_TOPIC` | `WeatherStation` |
| `MQTT_STALE_MS` | `90000` (sin mensaje → UI “stale”) |

ClientId: `HOSTNAME` + chipId (ej. `weather-indoor-01-a1b2c3`).

## Campos usados del JSON

| Campo | Uso en UI |
|-------|-----------|
| `temp_main` (fallback `temp_aht` / `temp_bmp`) | Temp LCD + OLED |
| `humidity` | Humedad |
| `pressure` | Presión |
| `ext_desc`, `ext_temp`, `ext_ok` | Condición OWM en LCD |
| `bmp180_ok`, `aht10_ok` | (reservado / diagnóstico) |
| `ip`, `firmware` | Guardados; OLED prioriza IP local del indoor |

Payload completo: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).

## Probar sin indoor

```powershell
mosquitto_sub -h demo.tbmq.io -p 1883 -u demo -t WeatherStation -v
```

## Serial

Logs: `[MQTT] connected`, `[MQTT] subscribed`, `[MQTT] data`, `[MQTT] fail`, `[MQTT] wait wifi`.
