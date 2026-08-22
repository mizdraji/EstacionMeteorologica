# Indoor display

Firmware **0.3.9** del nodo **interior** (ESP32 **TTGO LoRa32 V1**, env PlatformIO `ttgo`): se suscribe al MQTT del outdoor. **UI = LCD ST7789 240×240** (cinco vistas rotativas, incluida **Sistema**) **+ MAX7219** (hora NTP). El OLED físico fue retirado: no hay código ni librería SSD1306; otras placas ESP32 también van **siempre sin OLED**. LoRa no se usa (RST=GPIO14 LOW).

## Hardware

Ver [`docs/pinout.md`](docs/pinout.md). Board PlatformIO: `ttgo-lora32-v1` (env `ttgo`).

### LCD GMT130-V1.0 (HW SPI propio, CS=-1)

| Señal | GPIO |
|-------|------|
| CS | — (`-1`) |
| DC | 4 |
| RST | 16 |
| SCK | 23 |
| MOSI | 13 |
| VCC / BLK | 3V3 |

```cpp
SPIClass lcdSpi(HSPI);  // no VSPI (defaults 18/5 = MAX)
lcdSpi.begin(23, -1, 13, -1);
Adafruit_ST7789 tft(&lcdSpi, -1, 4, 16);
tft.init(240, 240, SPI_MODE3);
```

No usar el bus LoRa ni VSPI para el LCD (HSPI en 23/13). LoRa: **RST GPIO14 = LOW**.

### MAX7219 (LedControl)

Librería: **LedControl** (`lib/LedControl`) — ctor `LedControl(data/DIN, clk, cs, n)`.
Pines validados en este TTGO:

| Señal | GPIO | LedControl |
|-------|------|------------|
| DIN | 21 | dataPin |
| CLK | 18 | clkPin |
| CS | 5 | csPin |

Nota: 18/5 son también LoRa CS/SCK en la PCB; el radio queda en reset (RST=14 LOW).

```powershell
pio run -e indoor_max_test -t upload
pio device monitor -e indoor_max_test
```
## Flashear

Diagnóstico **solo LCD** (recomendado primero):

```powershell
cd indoor
pio run -e indoor_lcd_test
pio run -e indoor_lcd_test -t erase
pio run -e indoor_lcd_test -t upload
pio device monitor -e indoor_lcd_test
```

Firmware completo (default = `ttgo`):

```powershell
cd indoor
pio run
pio run -t erase
pio run -t upload
pio device monitor -b 115200
```

Explícito:

```powershell
pio run -e ttgo -t upload
pio device monitor -e ttgo
```

Primera vez WiFi: AP `weather-indoor-01` → http://192.168.4.1.

## Qué se ve (0.3.9)

| Display | Contenido |
|---------|-----------|
| **LCD** (UI principal) | Cinco vistas a pantalla completa, rotación cada ~7 s (`LCD_VIEW_ROTATE_MS`): temperatura, humedad (con barra), presión, condición OWM (o WiFi/MQTT si no hay OWM), **Sistema** (uptime `HH:MM:SS` o `Dd HH:MM:SS` + free heap en KB/bytes; barra/texto rojo y `LOW` si heap &lt; 48 KB). Chrome: “Indoor” / “Sin MQTT”, reloj `HH:MM` si NTP OK, puntos de vista abajo. |
| **MAX7219** | Hora NTP `HH-MM-SS` (pool South America, UTC-3). Hasta sincronizar: dashes. |
| **Serial** | Cada ~20 s: `[SYS] uptime=… heap=…` (añade `LOW` si el heap está bajo). |

WiFi: portal AP **`weather-indoor-01`** → http://192.168.4.1. MQTT topic `WeatherStation` en `demo.tbmq.io`.

LedControl en ESP32: usar `lib/LedControl` (parche `pgmspace.h`); el paquete wayoda del registry crashea/no compila. No forzar LoRa CS HIGH: GPIO18 es CLK del MAX.

## Estabilidad / heap

**Free heap** es la RAM dinámica que el ESP32 aún puede asignar (`ESP.getFreeHeap()`). Baja al conectar WiFi/MQTT y al usar buffers JSON/GFX; no es “RAM total libre” de la placa.

Rangos **aproximados** (ESP32 Arduino, ~320 KB DRAM; no hay una medición de runtime fijada en este repo):

| Situación | Free heap típico |
|-----------|------------------|
| Boot / sketch liviano | ~200–280 KB |
| Con WiFi + MQTT + LCD/MAX (este FW) | suele quedar **por encima de ~100 KB**; puede bajar más según buffers WiFi |
| Alerta en LCD/Serial | **&lt; 48 KB** (`HEAP_WARN_BYTES` en `Config.h`: `48UL * 1024UL`) |

Qué mirar:

- **Estable**: heap que oscila un poco pero no cae en tendencia, y **uptime que crece** (vista Sistema o `[SYS]` cada ~20 s) → no hay crash loop.
- **Preocupante**: heap **&lt; 48 KB** (`LOW` en LCD y en Serial), o una **caída continua** (p. ej. decenas de KB cada minuto) aunque aún esté por encima del umbral.
- **Crash / panic**: al arrancar, Serial imprime `Reset: N`. El valor **4** es `ESP_RST_PANIC` (excepción). Si se repite con uptime que no pasa de unos segundos, hay bucle de reinicio.

El umbral de 48 KB es una **alerta temprana**, no el momento exacto del crash. Un heap “bueno” para este nodo es del orden de **cientos de KB**, no de decenas.

## Docs

- [`docs/overview.md`](docs/overview.md)
- [`docs/pinout.md`](docs/pinout.md)
- [`docs/mqtt.md`](docs/mqtt.md)
