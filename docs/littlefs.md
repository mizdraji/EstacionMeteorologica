# LittleFS

Sistema de archivos en flash del ESP8266. En este proyecto guarda la **UI web** (`index.html`, `style.css`, `script.js`) que sirve el firmware.

## Rol en el proyecto

- Carpeta del repo: `data/` (raíz PlatformIO) → se sube a la partición LittleFS del ESP.
- Código: `src/network/WebServerManager.cpp` monta LittleFS y sirve esos archivos en el puerto 80.
- **No** se usa para credenciales WiFi (eso es WiFiManager / flash del SDK; ver [wifi-manager.md](wifi-manager.md)).

Si no subís `data/`, la API puede responder pero la página en `/` falla o queda vacía.

## PlatformIO (recomendado)

En `platformio.ini` está `board_build.filesystem = littlefs`.

```powershell
# Cerrar el monitor serie antes de subir FS
pio run -t uploadfs
```

Ver también [platformio.md](platformio.md).

No hace falta recompilar el firmware solo para cambiar HTML/CSS/JS: alcanza con `uploadfs`.

## Arduino IDE (legado)

Si aún usás el IDE en otro checkout antiguo:

### Arduino IDE 2.x — plugin

1. Descargá el `.vsix` desde releases de  
   [earlephilhower/arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload/releases)
2. En Arduino IDE 2: **Ctrl+Shift+P** → `Install from VSIX...`
3. Uso: **Ctrl+Shift+P** → `Upload LittleFS to Pico/ESP8266`

### Arduino IDE 1.8

Instalá el plugin “ESP8266 LittleFS Data Upload” y usá  
**Tools → ESP8266 LittleFS Data Upload**.

## Cómo subir `data/`

1. Placa: **NodeMCU 1.0 (ESP-12E Module)** / env `nodemcuv2` en PlatformIO.
2. Elegí el **puerto COM** correcto.
3. **Cerrá el Serial Monitor** (si está abierto, el upload suele fallar).
4. Ejecutá `pio run -t uploadfs` (o el comando del plugin en Arduino IDE).
5. Esperá el mensaje de éxito en la consola.

## Tamaño de partición

El board `nodemcuv2` en PlatformIO usa un esquema con espacio para FS por defecto. Si el upload falla por tamaño, revisá `board_build.ldscript` / documentación del core ESP8266.

## Troubleshooting

| Síntoma | Qué revisar |
|---------|-------------|
| Upload falla con puerto ocupado | **Cerrar Serial Monitor** y reintentar |
| Web en `/` rota, API OK | No se subió `data/` o se borró la flash (Erase All) |
| `[WEB] LittleFS mount failed` | Partición FS incorrecta o flash borrada sin re-subir FS |
| `pio` no encontrado | Instalar PlatformIO Core o usar la extensión; ver [platformio.md](platformio.md) |

## Contenido típico de `data/`

- `index.html` — UI (reloj, gráficos)
- `style.css`
- `script.js` — fetch a `/api/data`, `/api/status`, `/api/history`
