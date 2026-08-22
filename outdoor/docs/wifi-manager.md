# WiFiManager

Configuración de WiFi **sin hardcodear** SSID/password en el firmware.

## Qué es

[WiFiManager](https://github.com/tzapu/WiFiManager) (autor **tzapu**) intenta conectar con credenciales ya guardadas en la flash del ESP. Si no hay red o falla la conexión, abre un **portal de configuración** (Access Point + página web) para elegir la red desde el teléfono o la PC.

En este proyecto vive en `src/network/StationWiFi.cpp`. Web, OTA, mDNS y NTP arrancan **después** de tener WiFi.

## Librería

Con PlatformIO está en `lib_deps` de `platformio.ini` (tzapu/WiFiManager). Detalle de build: [platformio.md](platformio.md).

## Primer uso (portal AP)

1. Subí el firmware y abrí Serial a **115200** baud (mensajes `[WIFI] ...`).
2. Si no hay credenciales válidas, el ESP crea un AP con el nombre de `HOSTNAME` en `src/Config.h` (por defecto `weather-station-01`).
3. Conectate a ese AP desde el teléfono/PC.
4. Abrí el navegador en [http://192.168.4.1](http://192.168.4.1).
5. Elegí tu red WiFi, ingresá la contraseña y guardá.
6. El ESP conecta; recién ahí levantan web / OTA / NTP.

Los siguientes boots usan la red guardada (sin portal), salvo que falle la conexión.

## Timeouts (`src/Config.h`)

| Macro | Default | Rol |
|-------|---------|-----|
| `WIFI_CONNECT_TIMEOUT_SEC` | 20 | Espera al conectar con credenciales guardadas antes de abrir el portal |
| `WIFI_CONFIG_PORTAL_TIMEOUT_SEC` | 180 | Tiempo máximo del portal AP; si expira, no bloquea forever |
| `WIFI_INTERVAL_MS` | 5000 | Reintentos en background si quedó sin WiFi |

Si el portal expira sin configurar, los sensores siguen activos y `StationWiFi::update()` reintenta con las credenciales persistidas (`WiFi.begin()`).

## Relación con `secrets.h` / OTA

- **WiFi:** no va en `secrets.h`. Lo gestiona WiFiManager.
- **OTA:** sí. En `src/secrets.h` van `OTA_PASSWORD` y las credenciales MQTT (`MQTT_HOST` / `MQTT_USER` / `MQTT_PASSWORD` / `MQTT_TOPIC`; plantilla: `src/secrets.h.example`).
- `src/secrets.h` está en `.gitignore` y no se versiona.

## Dónde se guardan las credenciales

En la **flash del SDK WiFi** del ESP8266, no en LittleFS. No hay conflicto con la UI en `data/` (ver [littlefs.md](littlefs.md)).

## Borrar o cambiar de red

Opciones:

1. Volver a abrir el portal (si falla la red guardada y aún no expiró el timeout), o
2. Al subir el sketch: *Erase Flash → All Flash Contents* (borra también LittleFS; hay que volver a subir `data/`), o
3. Un sketch/helper que llame `WiFiManager::resetSettings()` / `WiFi.disconnect(true)`.

Este firmware **no** incluye botón físico de reset de credenciales (posible mejora futura).

## Serial útil

```
[WIFI] Intentando credenciales guardadas...
[WIFI] Si falla, portal AP: weather-station-01
[WIFI] Abrí http://192.168.4.1 (timeout 180 s)
[WIFI] Conectado. IP: ...
[WIFI] Servicios web/OTA/mDNS iniciados
```
