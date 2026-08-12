# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, DHT11, MAX7219 (hora NTP) y OLED de diagnóstico.

## Librerías requeridas

- ESP8266 board support (NodeMCU 1.0)
- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg)

## Configuración

1. Copiá `secrets.h.example` → `secrets.h`
2. Completá WiFi y password OTA en `secrets.h` (ese archivo **no** se sube a git)
3. Ajustá en `Config.h` el resto: GPIO, NTP, `SEA_LEVEL_PRESSURE_HPA`, hostname

## GPIO (kit ideaspark + MAX7219)

| Señal | Pin | GPIO |
|-------|-----|------|
| BMP180 SDA | D2 | 4 |
| BMP180 SCL | D1 | 5 |
| OLED SDA | D6 | 12 |
| OLED SCL | D5 | 14 |
| DHT11 | D7 | 13 |
| MAX7219 DIN | D8 | 15 |
| MAX7219 CLK | D4 | 2 |
| MAX7219 CS | D0 | 16 |

El MAX7219 muestra solo la hora local (`HH-MM-SS`) sincronizada por NTP (`3.south-america.pool.ntp.org`, UTC-3). Hasta sincronizar muestra guiones. La misma hora se muestra en la web.

## Compilar

1. Abrir `EstacionMeteorologica.ino`
2. Placa: NodeMCU 1.0 (ESP-12E Module)
3. Instalar librerías **TaskScheduler** y **NTPClient**
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
