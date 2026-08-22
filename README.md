# Estación Meteorológica

Dos firmwares en un solo repositorio:

| Carpeta | Rol |
|---------|-----|
| [`outdoor/`](outdoor/) | Nodo **exterior** (ESP8266): sensores (AHT10 + BMP180), web local, OTA, publica MQTT |
| [`indoor/`](indoor/) | Nodo **interior** (ESP32 TTGO LoRa32 V1, fw **0.3.9**): suscribe MQTT; UI = LCD ST7789 240×240 (5 vistas) + MAX7219 (hora NTP). Sin OLED. |

## Flujo de datos

```
[outdoor]  --publica JSON-->  [broker MQTT]  --suscribe-->  [indoor]
              topic WeatherStation              demo.tbmq.io
                                                    │
                                    ┌───────────────┴───────────────┐
                                    ▼                               ▼
                                 MAX7219                          LCD
                                 (hora NTP)                    (UI clima)
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

### Interior (env default `ttgo`)

```powershell
cd indoor
pio run                    # env ttgo
pio run -t upload
pio device monitor -b 115200
```

Diagnóstico: `pio run -e indoor_lcd_test -t upload` · `pio run -e indoor_max_test -t upload`.

Pinout (LCD HSPI 23/13 DC=4 RST=16 CS=-1; MAX DIN=21 CLK=18 CS=5), AP `weather-indoor-01`, MQTT `WeatherStation`: [`indoor/README.md`](indoor/README.md).

## Secrets

No versionar credenciales reales. Plantillas:

- `outdoor/src/secrets.h.example` → `outdoor/src/secrets.h` (OTA + OWM)
- `indoor/src/secrets.h.example` → `indoor/src/secrets.h` (opcional; WiFi vía WiFiManager)
