# MQTT (publicación telemetría)

El firmware **outdoor** publica un JSON compacto al broker **demo público** de ThingsBoard MQTT (`demo.tbmq.io`). No es un broker de producción: cualquiera puede suscribirse al topic. El nodo [`indoor/`](../../indoor/) (WIP) se suscribirá a este mismo flujo; ver [arquitectura](../../docs/architecture.md).

## Parámetros (`src/Config.h`)

| Define | Valor demo |
|--------|------------|
| `MQTT_HOST` | `demo.tbmq.io` |
| `MQTT_PORT` | `1883` |
| `MQTT_USER` | `demo` |
| `MQTT_PASSWORD` | `""` (string vacío; no `nullptr`) |
| `MQTT_TOPIC` | `WeatherStation` |
| `MQTT_INTERVAL_MS` | `30000` (alineado al historial web) |
| `MQTT_TASK_INTERVAL_MS` | `1000` (loop / reconnect no bloqueante) |

ClientId: `HOSTNAME` + chipId parcial (ej. `weather-station-01-a1b2c3`).

Si WiFi cae, no se spamea reconnect; al volver a conectar se usa backoff suave (1s → 60s).

## Probar con mosquitto_sub

```powershell
mosquitto_sub -h demo.tbmq.io -p 1883 -u demo -t WeatherStation -v
```

Password vacía: omití `-P` o usá `-P ""` según tu cliente.

Cada ~30 s deberías ver un mensaje en el topic `WeatherStation`.

## Ejemplo de payload

```json
{"temp_main":24.5,"temp_aht":24.5,"temp_bmp":25.1,"humidity":62.0,"pressure":1012.3,"altitude":45.0,"bmp180_ok":true,"aht10_ok":true,"ext_temp":23.8,"ext_humidity":65.0,"ext_ok":true,"ext_desc":"nubes","ip":"192.168.1.50","uptime":3600,"firmware":"1.6.1","heap":28000,"rssi":-55}
```

Campos inválidos van como `null`. `ext_*` refleja OpenWeatherMap cuando está configurado.

## Serial

Logs: `[MQTT] connected`, `[MQTT] publish`, `[MQTT] fail`, `[MQTT] wait wifi`.

## Librería

`knolleary/PubSubClient` en `platformio.ini` (`lib_deps`).
