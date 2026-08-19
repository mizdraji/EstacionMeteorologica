# Estación Meteorológica (monorepo)

Dos firmwares en un solo repositorio:

| Carpeta | Rol |
|---------|-----|
| [`outdoor/`](outdoor/) | Nodo **exterior**: sensores (AHT10 + BMP180), web local, OTA, MQTT |
| [`indoor/`](indoor/) | Nodo **interior**: visualización (WIP) — consume datos vía MQTT |

## Flujo de datos (alto nivel)

```
[outdoor]  --publica JSON-->  [broker MQTT]  --suscribe-->  [indoor]
                                 topic p.ej. WeatherStation
```

El exterior mide y publica telemetría. El interior (en desarrollo) se suscribe al mismo broker/topic y muestra los datos en display o UI local. Detalle compartido: [`docs/architecture.md`](docs/architecture.md).

## Empezar

- **Exterior (listo para build):** abrí la carpeta `outdoor/` como proyecto PlatformIO, o desde la raíz:

  ```powershell
  cd outdoor
  pio run
  ```

  Guía completa: [`outdoor/README.md`](outdoor/README.md).

- **Interior (scaffold):** [`indoor/README.md`](indoor/README.md) — stub Serial, board aún por definir.

## Secrets

No versionar credenciales reales. Plantillas:

- `outdoor/src/secrets.h.example` → copiar a `outdoor/src/secrets.h`
- (opcional) `indoor/src/secrets.h.example` cuando el indoor necesite claves
