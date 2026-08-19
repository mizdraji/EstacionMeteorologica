# Indoor display

Firmware del nodo **interior** (ESP8266 NodeMCU / placa ideaspark con OLED): se suscribe al MQTT del outdoor y muestra datos en **MAX7219** (hora), **OLED** (diagnóstico) y **LCD ST7789 240×240** (UI principal).

## Hardware

Ver [`docs/pinout.md`](docs/pinout.md). Resumen LCD elegido:

| Señal LCD | GPIO | NodeMCU |
|-----------|------|---------|
| SCK | 13 | D7 |
| MOSI (SDA) | 5 | D1 |
| DC | 4 | D2 |
| RST | 0 | D3 |
| CS | — | atar a **GND** |
| BLK | — | atar a **3V3** |

OLED: D6/D5. MAX7219: D8/D4/D0.

## Flashear

1. Abrí la carpeta `indoor/` como proyecto PlatformIO (o desde la raíz del monorepo).
2. (Opcional) Copiá `src/secrets.h.example` → `src/secrets.h` solo si querés overrides; **no** hace falta para WiFi.
3. Compilar y subir:

```powershell
cd indoor
pio run
pio run -t upload
pio device monitor
```

4. Primera vez WiFi: el ESP abre AP `weather-indoor-01` → http://192.168.4.1 (timeout 180 s).

## Qué se ve en cada pantalla

| Display | Contenido |
|---------|-----------|
| **MAX7219** | Hora local `HH-MM-SS` vía NTP (UTC-3). Sin sync: guiones. |
| **OLED 128×64** | Temp + humedad, presión, WiFi/RSSI, IP, estado MQTT + heap libre. |
| **LCD 240×240** | Temp grande, humedad, presión, condición OWM (`ext_desc`) si viene en el JSON. |

## MQTT

Mismo broker/topic que outdoor: [`docs/mqtt.md`](docs/mqtt.md).

## Limitaciones (ESP8266 + 3 displays)

- **RAM (~80 KB)**: WiFiManager + PubSubClient + ArduinoJson + buffer OLED (~1 KB) + SPI soft al LCD dejan poco margen. Evitá framebuffers grandes; el LCD dibuja con Adafruit GFX sin FB de pantalla completa.
- **Soft SPI** al ST7789 es más lento que HW SPI; el refresh del LCD está throttled (~2 s).
- **GPIO0 (D3) = LCD RST**: en boot debe quedar HIGH (reset inactivo). No atar RST a GND.
- **Sin OTA** en v0.1 (prioridad: displays + MQTT).
- Broker demo público: cualquiera puede publicar en el topic.

## Docs

- [`docs/overview.md`](docs/overview.md)
- [`docs/pinout.md`](docs/pinout.md)
- [`docs/mqtt.md`](docs/mqtt.md)
