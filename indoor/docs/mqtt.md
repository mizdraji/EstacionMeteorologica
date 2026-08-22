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

| Campo | Uso en UI (LCD 0.3.7; OLED off) |
|-------|-----------|
| `temp_main` (fallback `temp_aht` / `temp_bmp`) | Vista temperatura |
| `humidity` | Vista humedad |
| `pressure` | Vista presión |
| `ext_desc`, `ext_temp`, `ext_humidity`, `ext_ok` | Vista OWM (si no hay OWM: WiFi/MQTT) |
| `bmp180_ok`, `aht10_ok` | (reservado) |
| `ip`, `firmware`, `uptime`, `heap` | Pueden llegar en el JSON outdoor; **no** se muestran en LCD |

Payload completo: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).

## Probar sin indoor

```powershell
mosquitto_sub -h demo.tbmq.io -p 1883 -u demo -t WeatherStation -v
```

## Serial

Logs: `[MQTT] connected`, `[MQTT] subscribed`, `[MQTT] data`, `[MQTT] fail`, `[MQTT] wait wifi`.
