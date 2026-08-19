# Indoor display (WIP)

Firmware del nodo **interior**: visualizar datos de la estación exterior (MQTT → display / UI).

Aún no hay hardware ni board definitivos. Este directorio es un scaffold para empezar.

## Qué hará

Ver [`docs/overview.md`](docs/overview.md).

## Build (placeholder)

```powershell
cd indoor
# pio run   # cuando haya env definido en platformio.ini
```

Abrí `indoor/` como proyecto PlatformIO cuando elijas la placa.

## Estado actual

- `src/main.cpp`: stub `setup`/`loop` con mensaje Serial `Indoor display – WIP`
- `platformio.ini`: entorno mínimo comentado / TBD (ESP32 u otra placa)
- Secrets: opcional `src/secrets.h.example` (no hay claves aún)
