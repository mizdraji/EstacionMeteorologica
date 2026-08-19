# Estación Meteorológica (monorepo)

Dos firmwares en un solo repositorio:

| Carpeta | Rol |
|---------|-----|
| [`outdoor/`](outdoor/) | Nodo **exterior**: sensores (AHT10 + BMP180), web local, OTA, publica MQTT |
| [`indoor/`](indoor/) | Nodo **interior**: suscribe MQTT y muestra en MAX7219 + OLED + LCD 240×240 |

## Flujo de datos

```
[outdoor]  --publica JSON-->  [broker MQTT]  --suscribe-->  [indoor]
              topic WeatherStation              demo.tbmq.io
                                                    │
                                    ┌───────────────┼───────────────┐
                                    ▼               ▼               ▼
                                 MAX7219          OLED            LCD
                                 (hora NTP)    (diagnóstico)   (UI clima)
```

Detalle: [`docs/architecture.md`](docs/architecture.md).

## Empezar

### Exterior

```powershell
cd outdoor
pio run
pio run -t upload
```

Guía: [`outdoor/README.md`](outdoor/README.md).

### Interior

```powershell
cd indoor
pio run
pio run -t upload
```

Pinout LCD / flasheo / pantallas: [`indoor/README.md`](indoor/README.md).

## Secrets

No versionar credenciales reales. Plantillas:

- `outdoor/src/secrets.h.example` → `outdoor/src/secrets.h` (OTA + OWM)
- `indoor/src/secrets.h.example` → `indoor/src/secrets.h` (opcional; WiFi vía WiFiManager)
