# Indoor display

Firmware **0.3.7** del nodo **interior** (ESP32 **TTGO LoRa32 V1**, env PlatformIO `ttgo`): se suscribe al MQTT del outdoor y muestra datos en **MAX7219** (hora NTP) y **LCD ST7789 240×240** (cuatro vistas rotativas). OLED deshabilitado (`OLED_ENABLED=0`); LoRa no se usa (RST=GPIO14 LOW).

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

## Qué se ve (0.3.7)

| Display | Contenido |
|---------|-----------|
| **LCD** | Cuatro vistas a pantalla completa, rotación cada ~7 s (`LCD_VIEW_ROTATE_MS`): temperatura, humedad (con barra), presión, condición OWM (o WiFi/MQTT si no hay OWM). Chrome: “Indoor” / “Sin MQTT”, reloj `HH:MM` si NTP OK, puntos de vista abajo. **No** muestra uptime ni heap. |
| **MAX7219** | Hora NTP `HH-MM-SS` (pool South America, UTC-3). Hasta sincronizar: dashes. |
| **OLED** | Deshabilitado (`OLED_ENABLED=0`) |

WiFi: portal AP **`weather-indoor-01`** → http://192.168.4.1. MQTT topic `WeatherStation` en `demo.tbmq.io`.

LedControl en ESP32: usar `lib/LedControl` (parche `pgmspace.h`); el paquete wayoda del registry crashea/no compila. No forzar LoRa CS HIGH: GPIO18 es CLK del MAX.

## Docs

- [`docs/overview.md`](docs/overview.md)
- [`docs/pinout.md`](docs/pinout.md)
- [`docs/mqtt.md`](docs/mqtt.md)
