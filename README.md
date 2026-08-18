# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, DHT11, MAX7219 (hora NTP) y OLED de diagnóstico.

Proyecto gestionado con **PlatformIO** (placa NodeMCU 1.0).

## Documentación

| Tema | Archivo |
|------|---------|
| PlatformIO (build / upload / FS) | [docs/platformio.md](docs/platformio.md) |
| WiFi (portal WiFiManager) | [docs/wifi-manager.md](docs/wifi-manager.md) |
| LittleFS (UI web) | [docs/littlefs.md](docs/littlefs.md) |
| Pinout y conexiones | [docs/pinout.md](docs/pinout.md) |

## Librerías

Declaradas en `platformio.ini` (`lib_deps`); PlatformIO las instala solo:

- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg / arduino-libraries)
- WiFiManager (tzapu) — detalle en [docs/wifi-manager.md](docs/wifi-manager.md)

## Configuración rápida

1. Copiá `src/secrets.h.example` → `src/secrets.h`
2. Completá solo `OTA_PASSWORD` (WiFi **no** va en secrets; ver [docs/wifi-manager.md](docs/wifi-manager.md))
3. Ajustá GPIO, NTP, hostname y timeouts en `src/Config.h`

`src/secrets.h` no se versiona (`.gitignore`).

## Compilar y subir (PlatformIO)

Desde la raíz del repo:

```powershell
pio run                 # compilar
pio run -t upload       # firmware
pio run -t uploadfs     # carpeta data/ → LittleFS
pio device monitor      # serial 115200
```

UI (hormiga → PROJECT TASKS), Erase Flash y prueba limpia: [docs/platformio.md](docs/platformio.md). LittleFS: [docs/littlefs.md](docs/littlefs.md).

## Acceso

- `http://<IP>/`
- `http://weather-station-01.local`
- API: `/api/data`, `/api/status`, `/api/history`

Historial: ring buffer en RAM (`HISTORY_CAPACITY` × `HISTORY_INTERVAL_MS`); se pierde al reiniciar. La web dibuja temp/humedad/presión en canvas (sin CDN).

## OTA

Host `weather-station-01`, contraseña en `OTA_PASSWORD` (`src/secrets.h`).

## Serial

115200 baud.
