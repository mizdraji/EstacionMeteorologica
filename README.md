# Estación Meteorológica Local (ESP8266)

Firmware modular para medición local con BMP180, DHT11, MAX7219 (hora NTP) y OLED de diagnóstico.

## Documentación

| Tema | Archivo |
|------|---------|
| WiFi (portal WiFiManager) | [docs/wifi-manager.md](docs/wifi-manager.md) |
| LittleFS (UI web / plugin) | [docs/littlefs.md](docs/littlefs.md) |
| Pinout y conexiones | [docs/pinout.md](docs/pinout.md) |

## Librerías

- ESP8266 board support (NodeMCU 1.0)
- TaskScheduler (arkhipenko)
- NTPClient (Fabrice Weinberg)
- WiFiManager (tzapu) — detalle en [docs/wifi-manager.md](docs/wifi-manager.md)

## Configuración rápida

1. Copiá `secrets.h.example` → `secrets.h`
2. Completá solo `OTA_PASSWORD` (WiFi **no** va en secrets; ver [docs/wifi-manager.md](docs/wifi-manager.md))
3. Ajustá GPIO, NTP, hostname y timeouts en `Config.h`

`secrets.h` no se versiona (`.gitignore`).

## Compilar y subir

1. Abrir `EstacionMeteorologica.ino`
2. Placa: NodeMCU 1.0 (ESP-12E Module)
3. Instalar las librerías de arriba
4. Subir sketch
5. Subir carpeta `data/` a LittleFS — [docs/littlefs.md](docs/littlefs.md)

Arduino IDE 1.8 solo compila `.cpp` en la raíz del sketch; por eso existe `AllModules.cpp`.

## Acceso

- `http://<IP>/`
- `http://weather-station-01.local`
- API: `/api/data`, `/api/status`, `/api/history`

Historial: ring buffer en RAM (`HISTORY_CAPACITY` × `HISTORY_INTERVAL_MS`); se pierde al reiniciar. La web dibuja temp/humedad/presión en canvas (sin CDN).

## OTA

Host `weather-station-01`, contraseña en `OTA_PASSWORD` (`secrets.h`).

## Serial

115200 baud.
