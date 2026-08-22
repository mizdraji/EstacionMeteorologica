# Estación exterior (outdoor)

Firmware modular para estación **exterior** en NodeMCU ESP8266 (sin OLED ni display 7 segmentos): BMP180 + AHT10, web local, WiFiManager, OTA, NTP, comparación con OpenWeatherMap y publicación MQTT.

Proyecto PlatformIO (placa NodeMCU 1.0). Pinout: [docs/pinout.md](docs/pinout.md).

Parte del monorepo: ver [README raíz](../README.md) e [arquitectura](../docs/architecture.md).

## Sensores (sin pantallas)

- **AHT10** — temperatura y humedad (I2C)
- **BMP180** — temperatura, presión y altitud (I2C)
- Sin OLED / MAX7219: la visualización local queda en el nodo [`indoor/`](../indoor/) (ESP32 TTGO LoRa32, firmware indoor 0.3.7); aquí se publica por MQTT y web.

## Documentación

| Tema | Archivo |
|------|---------|
| PlatformIO (build / upload / FS) | [docs/platformio.md](docs/platformio.md) |
| WiFi (portal WiFiManager) | [docs/wifi-manager.md](docs/wifi-manager.md) |
| LittleFS (UI web) | [docs/littlefs.md](docs/littlefs.md) |
| OpenWeatherMap (clima externo) | [docs/openweathermap.md](docs/openweathermap.md) |
| MQTT (telemetría) | [docs/mqtt.md](docs/mqtt.md) |
| Pinout y conexiones | [docs/pinout.md](docs/pinout.md) |

## Librerías

Declaradas en `platformio.ini` (`lib_deps`); PlatformIO las instala solo:

- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg / arduino-libraries)
- WiFiManager (tzapu) — detalle en [docs/wifi-manager.md](docs/wifi-manager.md)
- ArduinoJson (bblanchon) — parse de OpenWeatherMap y payload MQTT
- PubSubClient (knolleary) — publicación MQTT; ver [docs/mqtt.md](docs/mqtt.md)

## Configuración rápida

1. Copiá `src/secrets.h.example` → `src/secrets.h`
2. Completá `OTA_PASSWORD` y `OPENWEATHERMAP_API_KEY` (WiFi **no** va en secrets; ver [docs/wifi-manager.md](docs/wifi-manager.md))
3. Ajustá GPIO, NTP, hostname, ubicación OWM (`OWM_CITY_ID` / lat-lon) e intervalos en `src/Config.h`

`src/secrets.h` no se versiona (`.gitignore` en la raíz del monorepo).

## Compilar y subir (PlatformIO)

Abrí la carpeta **`outdoor/`** como proyecto PlatformIO, o desde la raíz del monorepo:

```powershell
cd outdoor
pio run                 # compilar
pio run -t upload       # firmware
pio run -t uploadfs     # carpeta data/ → LittleFS
pio device monitor      # serial 115200
```

Tras cambios en `data/` **y** firmware: hace falta `upload` + `uploadfs`.

UI (hormiga → PROJECT TASKS), Erase Flash y prueba limpia: [docs/platformio.md](docs/platformio.md). LittleFS: [docs/littlefs.md](docs/littlefs.md).

## Acceso

- `http://<IP>/`
- `http://weather-station-01.local`
- API: `/api/data` (incluye campos `external_*`), `/api/status`, `/api/history`

Historial: ring buffer en RAM (`HISTORY_CAPACITY` × `HISTORY_INTERVAL_MS`); se pierde al reiniciar. La web dibuja temp/humedad/presión en canvas (sin CDN) y una tarjeta de comparación local vs OpenWeatherMap.

## OTA

Host `weather-station-01`, contraseña en `OTA_PASSWORD` (`src/secrets.h`).

## Serial

115200 baud.
