# Indoor — MQTT (suscripción)

El indoor se **suscribe** al mismo broker/topic que usa outdoor para publicar.

## Credenciales (`src/secrets.h`)

Host, usuario, password y topic **no** van en `Config.h`. Copiá `src/secrets.h.example` → `src/secrets.h` (no se versiona). Usá los mismos valores que outdoor.

| Define | Placeholder |
|--------|-------------|
| `MQTT_HOST` | `TU_MQTT_HOST` |
| `MQTT_USER` | `TU_MQTT_USER` |
| `MQTT_PASSWORD` | `TU_MQTT_PASSWORD` |
| `MQTT_TOPIC` | `TU_MQTT_TOPIC` |

## Parámetros (`src/Config.h`)

| Define | Valor |
|--------|-------|
| `MQTT_PORT` | `1883` |
| `MQTT_STALE_MS` | `30000` (sin mensaje → LCD muestra `-`; ajustable) |

ClientId: `HOSTNAME` + chipId (ej. `weather-indoor-01-a1b2c3`).

## Campos usados del JSON

| Campo | Uso en UI (LCD 0.3.9) |
|-------|-----------|
| `temp_main` (fallback `temp_aht` / `temp_bmp`) | Vista temperatura (`-` si MQTT stale) |
| `humidity` | Vista humedad (`-` si MQTT stale) |
| `pressure` | Vista presión (`-` si MQTT stale) |
| `ext_desc`, `ext_temp`, `ext_humidity`, `ext_ok` | Vista OWM (`-` si MQTT stale; si no hay OWM: WiFi/MQTT) |
| `bmp180_ok`, `aht10_ok` | (reservado) |
| `ip`, `firmware`, `uptime`, `heap` | Pueden llegar en el JSON outdoor; **no** se muestran en LCD |

Payload completo: [`../../outdoor/docs/mqtt.md`](../../outdoor/docs/mqtt.md).

## Probar sin indoor

```powershell
mosquitto_sub -h TU_MQTT_HOST -p 1883 -u TU_MQTT_USER -P TU_MQTT_PASSWORD -t TU_MQTT_TOPIC -v
```

## Serial

Logs: `[MQTT] connected`, `[MQTT] subscribed`, `[MQTT] data`, `[MQTT] fail`, `[MQTT] wait wifi`.
