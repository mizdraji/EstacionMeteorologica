# PlatformIO

Este proyecto se construye con [PlatformIO](https://platformio.org/) (framework Arduino, placa NodeMCU 1.0 / ESP8266).

## Requisitos

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html) (CLI `pio`) o la extensión PlatformIO en VS Code / Cursor
- Cable USB y drivers del puerto COM (CH340/CP210x según la placa)

## Estructura relevante

| Ruta | Rol |
|------|-----|
| `platformio.ini` | Entorno `nodemcuv2`, LittleFS, dependencias |
| `src/` | Firmware (`main.cpp` + módulos) |
| `src/secrets.h.example` | Plantilla → copiar a `src/secrets.h` |
| `data/` | UI web subida a LittleFS |
| `docs/` | Documentación |

## Primera configuración

1. Copiá `src/secrets.h.example` → `src/secrets.h`
2. Completá solo `OTA_PASSWORD` (WiFi lo gestiona WiFiManager; ver [wifi-manager.md](wifi-manager.md))
3. Ajustá pines / NTP / hostname en `src/Config.h` si hace falta

## Comandos (desde la raíz del repo)

```powershell
# Compilar
pio run

# Subir firmware
pio run -t upload

# Subir LittleFS (carpeta data/)
pio run -t uploadfs

# Monitor serie (115200)
pio device monitor
```

Puerto COM: si no se detecta solo, en `platformio.ini` agregá por ejemplo:

```ini
upload_port = COMx
monitor_port = COMx
```

## Dependencias (`lib_deps`)

- **TaskScheduler** (arkhipenko)
- **NTPClient** (arduino-libraries)
- **WiFiManager** (tzapu)

PlatformIO las descarga al primer `pio run` en `.pio/` (ignorado por git).

## LittleFS

`board_build.filesystem = littlefs` en `platformio.ini`. Detalle de la UI y troubleshooting: [littlefs.md](littlefs.md).

Tras un *Erase All* de la flash, volvé a subir firmware **y** `uploadfs`.

## Serial

Baud rate: **115200** (`monitor_speed` en `platformio.ini`).
