# LittleFS

Sistema de archivos en flash del ESP8266. En este proyecto guarda la **UI web** (`index.html`, `style.css`, `script.js`) que sirve el firmware.

## Rol en el proyecto

- Carpeta del repo: `data/` → se sube a la partición LittleFS del ESP.
- Código: `network/WebServerManager.cpp` monta LittleFS y sirve esos archivos en el puerto 80.
- **No** se usa para credenciales WiFi (eso es WiFiManager / flash del SDK; ver [wifi-manager.md](wifi-manager.md)).

Si no subís `data/`, la API puede responder pero la página en `/` falla o queda vacía.

## Arduino IDE 2.x — instalar el plugin

En IDE 2 no viene el uploader clásico de IDE 1.8. Usá el plugin de earlephilhower:

1. Descargá el `.vsix` desde releases de  
   [earlephilhower/arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload/releases)
2. En Arduino IDE 2: **Ctrl+Shift+P** (o Cmd+Shift+P) → `Install from VSIX...`
3. Elegí el archivo `.vsix` descargado
4. Reiniciá el IDE si hace falta

Uso: con el sketch abierto y la placa/puerto seleccionados →  
**Ctrl+Shift+P** → `Upload LittleFS to Pico/ESP8266`

(El comando sirve también para ESP8266 pese al nombre.)

## Arduino IDE 1.8 (breve)

Instalá el plugin “ESP8266 LittleFS Data Upload” (herramientas de la comunidad ESP8266) y usá el menú  
**Tools → ESP8266 LittleFS Data Upload**.

## Cómo subir `data/`

1. Placa: **NodeMCU 1.0 (ESP-12E Module)** (o la misma que usás para el sketch).
2. Elegí el **puerto COM** correcto.
3. **Cerrá el Serial Monitor** (si está abierto, el upload suele fallar).
4. Ejecutá el upload de LittleFS (IDE 2: comando de arriba; IDE 1.8: menú Tools).
5. Esperá el mensaje de éxito en la consola.

No hace falta recompilar el sketch solo para cambiar HTML/CSS/JS: alcanza con volver a subir LittleFS.

## Tamaño de partición

En Tools del core ESP8266, la opción **Flash Size** / esquema de partición debe dejar espacio para FS (p. ej. “4MB (FS:2MB…)” o similar). Si el FS es 0, el upload falla o no hay sitio para `data/`.

## Troubleshooting

| Síntoma | Qué revisar |
|---------|-------------|
| `Not found` / no aparece el comando | Plugin `.vsix` no instalado o IDE sin reiniciar |
| `No port specified` / no sube | Puerto COM no elegido; cable/driver; cerrar otros programas que usen el puerto |
| Upload falla con puerto ocupado | **Cerrar Serial Monitor** y reintentar |
| Web en `/` rota, API OK | No se subió `data/` o se borró la flash (Erase All) |
| `[WEB] LittleFS mount failed` | Partición FS incorrecta o flash borrada sin re-subir FS |

## Contenido típico de `data/`

- `index.html` — UI (reloj, gráficos)
- `style.css`
- `script.js` — fetch a `/api/data`, `/api/status`, `/api/history`
