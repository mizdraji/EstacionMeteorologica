# MQTT (publicación telemetría)

El firmware **outdoor** publica un JSON compacto al broker configurado en `src/secrets.h`. El nodo [`indoor/`](../../indoor/) se suscribe al mismo host y topic; ver [arquitectura](../../docs/architecture.md) e [`indoor/docs/mqtt.md`](../../indoor/docs/mqtt.md).

## Credenciales (`src/secrets.h`)

Host, usuario, password y topic **no** van en `Config.h`. Copiá `src/secrets.h.example` → `src/secrets.h` (no se versiona).

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
| `MQTT_INTERVAL_MS` | `30000` (alineado al historial web) |
| `MQTT_TASK_INTERVAL_MS` | `1000` (loop / reconnect no bloqueante) |

ClientId: `HOSTNAME` + chipId parcial (ej. `weather-station-01-a1b2c3`).

Si WiFi cae, no se spamea reconnect; al volver a conectar se usa backoff suave (1s → 60s).

## Probar con mosquitto_sub

```powershell
mosquitto_sub -h TU_MQTT_HOST -p 1883 -u TU_MQTT_USER -P TU_MQTT_PASSWORD -t TU_MQTT_TOPIC -v
```

Password vacía: omití `-P` o usá `-P ""` según tu cliente.

Cada ~`MQTT_INTERVAL_MS` deberías ver un mensaje en el topic configurado.

## Ejemplo de payload

```json
{"temp_main":24.5,"temp_aht":24.5,"temp_bmp":25.1,"humidity":62.0,"pressure":1012.3,"altitude":45.0,"bmp180_ok":true,"aht10_ok":true,"ext_temp":23.8,"ext_humidity":65.0,"ext_ok":true,"ext_desc":"nubes","ip":"192.168.1.50","uptime":3600,"firmware":"1.6.1","heap":28000,"rssi":-55}
```

Campos inválidos van como `null`. `ext_*` refleja OpenWeatherMap cuando está configurado.

## Serial

Logs: `[MQTT] connected`, `[MQTT] publish`, `[MQTT] fail`, `[MQTT] wait wifi`.

## Librería

`knolleary/PubSubClient` en `platformio.ini` (`lib_deps`).
