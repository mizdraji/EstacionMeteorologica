# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, DHT11, MAX7219 (hora NTP) y OLED de diagnóstico.

## Librerías requeridas

- ESP8266 board support (NodeMCU 1.0)
- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg)
- **WiFiManager** (tzapu) — portal de configuración WiFi

En Arduino Library Manager: buscar `WiFiManager` de **tzapu** e instalar.

## Configuración

1. Copiá `secrets.h.example` → `secrets.h`
2. Completá solo `OTA_PASSWORD` en `secrets.h` (ese archivo **no** se sube a git)
3. Ajustá en `Config.h` el resto: GPIO, NTP, `SEA_LEVEL_PRESSURE_HPA`, hostname, timeouts WiFi

**WiFi no se hardcodea.** Las credenciales las guarda WiFiManager en la flash del SDK WiFi (no en LittleFS).

## WiFi (portal de configuración)

Al boot el firmware intenta conectar con la red guardada. Si no hay credenciales o falla:

1. El ESP abre un AP llamado como `HOSTNAME` (por defecto `weather-station-01`)
2. Con tu teléfono/PC, conectate a ese AP
3. Abrí el navegador en `http://192.168.4.1`
4. Elegí tu red WiFi, ingresá la contraseña y guardá
5. El ESP reinicia/conecta; web, OTA y NTP arrancan **después** de tener WiFi

Serial (115200) muestra mensajes `[WIFI] ...` durante el proceso. El portal tiene timeout (`WIFI_CONFIG_PORTAL_TIMEOUT_SEC`, por defecto 180 s); si expira, los sensores siguen activos y se reintenta en background.

### Borrar / cambiar red WiFi

- Desde el portal de WiFiManager (si se vuelve a abrir), o
- Borrando la flash WiFi del ESP (p. ej. “Erase Flash: All Flash Contents” al subir, o un sketch que llame `WiFi.disconnect(true)` / `WiFiManager::resetSettings()`)

No hay botón físico de reset de credenciales en este firmware (TODO opcional a futuro).

### LittleFS vs WiFiManager

LittleFS (UI web en `data/`) y WiFiManager no compiten por el mismo almacenamiento: WiFiManager persiste SSID/password en la flash del SDK WiFi; LittleFS usa su partición FS. Podés subir `data/` con normalidad.

## Pinout de la placa

Placa ESP8266 + OLED 0.96" integrado (AI-Thinker ESP8266MOD). El OLED interno ya está cableado a **D6 (SDA)** y **D5 (SCL)**.

![Pinout ESP8266 OLED](docs/esp8266-oled-pinout.png)

### Conexiones del proyecto

| Módulo | Señal | Pin placa | GPIO | Notas |
|--------|-------|-----------|------|-------|
| OLED (integrado) | SDA | D6 | 12 | Ya soldado en la placa |
| OLED (integrado) | SCL | D5 | 14 | Ya soldado en la placa |
| BMP180 | SDA | D2 | 4 | Bus I2C propio |
| BMP180 | SCL | D1 | 5 | Bus I2C propio |
| BMP180 | VCC / GND | 3V3 / GND | — | |
| DHT11 | DATA | D7 | 13 | |
| DHT11 | VCC / GND | 3V3 / GND | — | |
| MAX7219 | DIN | D8 | 15 | |
| MAX7219 | CLK | D4 | 2 | |
| MAX7219 | CS | D0 | 16 | |
| MAX7219 | VCC / GND | 5V / GND | — | Preferible 5V en VCC |

**No uses** los pines de la derecha (`CLK`, `SD0`, `CMD`, `SD1`…): son del flash interno.

El MAX7219 muestra solo la hora local (`HH-MM-SS`) sincronizada por NTP (`3.south-america.pool.ntp.org`, UTC-3). Hasta sincronizar muestra guiones. La misma hora se muestra en la web.

## Compilar

1. Abrir `EstacionMeteorologica.ino`
2. Placa: NodeMCU 1.0 (ESP-12E Module)
3. Instalar librerías **TaskScheduler**, **NTPClient** y **WiFiManager** (tzapu)
4. Subir sketch

Arduino IDE 1.8 solo compila `.cpp` en la raiz del sketch. Por eso existe `AllModules.cpp`, que incluye los modulos en subcarpetas.

## LittleFS

Subir carpeta `data/` con plugin LittleFS Data Upload (incluye la UI con reloj y gráficos).

## Acceso

- `http://<IP>/`
- `http://weather-station-01.local`
- API: `/api/data`, `/api/status`, `/api/history`

### Historial / gráficos

- Ring buffer en RAM: `HISTORY_CAPACITY` muestras cada `HISTORY_INTERVAL_MS` (por defecto ~30 min).
- La web dibuja temperatura, humedad y presión en canvas (sin CDN).
- Se pierde al reiniciar el ESP.

## OTA

Host `weather-station-01`, contraseña en `OTA_PASSWORD` (`secrets.h`).

## Serial

115200 baud.
