# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, AHT10, MAX7219 (hora NTP) y OLED de diagnóstico. Compara lecturas locales con el clima del día vía OpenWeatherMap (consulta desde el ESP).

Proyecto gestionado con **PlatformIO** (placa NodeMCU 1.0).

## Documentación

| Tema | Archivo |
|------|---------|
| PlatformIO (build / upload / FS) | [docs/platformio.md](docs/platformio.md) |
| WiFi (portal WiFiManager) | [docs/wifi-manager.md](docs/wifi-manager.md) |
| LittleFS (UI web) | [docs/littlefs.md](docs/littlefs.md) |
| OpenWeatherMap (clima externo) | [docs/openweathermap.md](docs/openweathermap.md) |
| Pinout y conexiones | [docs/pinout.md](docs/pinout.md) |

## Librerías

Declaradas en `platformio.ini` (`lib_deps`); PlatformIO las instala solo:

- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg / arduino-libraries)
- WiFiManager (tzapu) — detalle en [docs/wifi-manager.md](docs/wifi-manager.md)
- ArduinoJson (bblanchon) — parse de OpenWeatherMap

## Configuración rápida

1. Copiá `src/secrets.h.example` → `src/secrets.h`
2. Completá `OTA_PASSWORD` y `OPENWEATHERMAP_API_KEY` (WiFi **no** va en secrets; ver [docs/wifi-manager.md](docs/wifi-manager.md))
3. Ajustá GPIO, NTP, hostname, ubicación OWM (`OWM_CITY_ID` / lat-lon) e intervalos en `src/Config.h`

`src/secrets.h` no se versiona (`.gitignore`).

## Compilar y subir (PlatformIO)

Desde la raíz del repo:

```powershell
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
