# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, DHT11, MAX7219 (hora NTP) y OLED de diagnóstico.

## Librerías requeridas

- ESP8266 board support (NodeMCU 1.0)
- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg)

## Configuración

Editar `Config.h`: WiFi, OTA, GPIO, NTP y `SEA_LEVEL_PRESSURE_HPA`.

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

El MAX7219 muestra solo la hora local (`HH-MM-SS`) sincronizada por NTP (`3.south-america.pool.ntp.org`, UTC-3). Hasta sincronizar muestra guiones.

## Compilar

1. Abrir `EstacionMeteorologica.ino`
2. Placa: NodeMCU 1.0 (ESP-12E Module)
3. Instalar librerías **TaskScheduler** y **NTPClient**
4. Subir sketch

Arduino IDE 1.8 solo compila `.cpp` en la raiz del sketch. Por eso existe `AllModules.cpp`, que incluye los modulos en subcarpetas.

## LittleFS

Subir carpeta `data/` con plugin LittleFS Data Upload.

## Acceso

- `http://<IP>/`
- `http://weather-station-01.local`
- API: `/api/data`, `/api/status`

## OTA

Host `weather-station-01`, contraseña en `OTA_PASSWORD`.

## Serial

115200 baud.
