# PlatformIO

Este proyecto se construye con [PlatformIO](https://platformio.org/) (framework Arduino, placa NodeMCU 1.0 / ESP8266).

## Requisitos

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html) (CLI `pio`) o la extensión PlatformIO en VS Code / Cursor
- Cable USB y drivers del puerto COM (CH340/CP210x según la placa)

## Estructura relevante

| Ruta | Rol |
|------|-----|
| `outdoor/platformio.ini` | Entorno `nodemcuv2`, LittleFS, dependencias |
| `outdoor/src/` | Firmware (`main.cpp` + módulos) |
| `outdoor/src/secrets.h.example` | Plantilla → copiar a `src/secrets.h` |
| `outdoor/data/` | UI web subida a LittleFS |
| `outdoor/docs/` | Documentación de este dispositivo |

Todas las rutas relativas (`src/`, `data/`, `platformio.ini`) se resuelven **desde `outdoor/`**.

## Primera configuración

1. Copiá `src/secrets.h.example` → `src/secrets.h`
2. Completá solo `OTA_PASSWORD` (WiFi lo gestiona WiFiManager; ver [wifi-manager.md](wifi-manager.md))
3. Ajustá pines / NTP / hostname en `src/Config.h` si hace falta

## UI de PlatformIO (VS Code / Cursor)

Icono de la **hormiga** (barra lateral) → **PROJECT TASKS** → `nodemcuv2`:

| Sección | Tarea | Qué hace |
|---------|-------|----------|
| General | **Build** | Compila el firmware |
| General | **Upload** | Sube el firmware |
| General | **Clean** | Borra artefactos de build locales (`.pio/build/…`). **No** toca la flash del ESP |
| General | **Monitor** | Serial a 115200 |
| Platform | **Upload Filesystem Image** | Sube la carpeta `data/` a LittleFS |
| Platform | **Erase Flash** | Borra **toda** la flash del ESP (firmware, FS, WiFi guardado, etc.) |

No confundir **Erase Flash** (flash del micro) con **Clean** (solo build local).

Cerrá el Monitor antes de Upload / Upload Filesystem Image / Erase Flash; si el puerto está ocupado, el comando falla.

## Comandos CLI (equivalentes)

Desde la carpeta **`outdoor/`** (proyecto PlatformIO de este dispositivo):

```powershell
cd outdoor              # si estás en la raíz del monorepo
pio run                 # Build
pio run -t upload       # Upload firmware
pio run -t uploadfs     # Upload Filesystem Image (carpeta data/)
pio run -t erase        # Erase Flash
pio device monitor      # Monitor (115200)
pio run -t clean        # Clean (solo build local)
```

Puerto COM: si no se detecta solo, en `platformio.ini` agregá por ejemplo:

```ini
upload_port = COMx
monitor_port = COMx
```

## Prueba limpia (orden recomendado)

Tras un erase o cuando quieras partir de flash vacía:

1. **Erase Flash** (`pio run -t erase`)
2. **Upload** del firmware (`pio run -t upload`)
3. **Upload Filesystem Image** (`pio run -t uploadfs`) — carpeta `data/`
4. **Monitor** (`pio device monitor`) — 115200

Notas tras **Erase Flash**:

- WiFiManager vuelve a pedir red (portal cautivo); las credenciales se borraron.
- LittleFS queda vacío: hay que volver a subir `data/` o la web en `/` no carga.

Detalle de LittleFS y troubleshooting: [littlefs.md](littlefs.md).

## Dependencias (`lib_deps`)

- **TaskScheduler** (arkhipenko)
- **NTPClient** (arduino-libraries)
- **WiFiManager** (tzapu)

PlatformIO las descarga al primer `pio run` en `.pio/` (ignorado por git).

## LittleFS

`board_build.filesystem = littlefs` en `platformio.ini`. Contenido de `data/`, plugins Arduino IDE y troubleshooting: [littlefs.md](littlefs.md).

## Serial

Baud rate: **115200** (`monitor_speed` en `platformio.ini`).
