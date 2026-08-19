# Indoor — pinout (ESP8266 NodeMCU)

Placa base: **ideaspark / NodeMCU** con OLED I2C integrado. Pines en `src/Config.h`.

## OLED integrado (I2C)

| Señal | GPIO | NodeMCU | Notas |
|-------|------|---------|-------|
| SDA | 12 | D6 | Bus I2C propio del OLED |
| SCL | 14 | D5 | |
| ADDR | — | 0x3C | |

## MAX7219 (8 dígitos, hora)

| Señal | GPIO | NodeMCU |
|-------|------|---------|
| DIN | 15 | D8 |
| CLK | 2 | D4 |
| CS | 16 | D0 |

**Boot / strapping:** GPIO15 debe estar **LOW** y GPIO2 **HIGH** al reset.
El firmware pone esos niveles *antes* de hablar con el MAX7219 y deja CLK en HIGH
tras cada `shiftOut` (que lo deja LOW). Si hay boot loop (`ets Jan 8 2013` a 74880),
desconectá DIN/CLK o usá `pio run -e indoor_minimal`.

## LCD IPS 240×240 ST7789 (SPI bit-bang)

Elegido para **no chocar** con OLED (D5/D6) ni MAX7219 (D0/D4/D8).

| Señal módulo | GPIO | NodeMCU | Cableado |
|--------------|------|---------|----------|
| GND | — | GND | |
| VCC | — | 3V3 | (módulo 3.3 V) |
| SCK | **13** | **D7** | |
| SDA (MOSI) | **5** | **D1** | |
| RES (RST) | **0** | **D3** | Ver nota boot |
| DC | **4** | **D2** | |
| CS | — | — | **Atar a GND** (siempre seleccionado; `LCD_CS_PIN = -1`) |
| BLK | — | — | **Atar a 3V3** (backlight siempre on) |

### Por qué este mapeo

- D5/D6 ocupados por OLED → no se puede usar SPI “estándar” del NodeMCU en esos pines.
- D0/D4/D8 = MAX7219.
- Libres útiles: D1, D2, D3, D7.
- **D3 = GPIO0**: pin de boot. El LCD RST inactivo es HIGH; el módulo normalmente tiene pull-up. **No** forzar RST a LOW en el encendido. `Adafruit_ST7789::init()` puede pulsar RST por software *después* del boot — eso es OK.
- BLK y CS no usan GPIO extras: ahorran pines y evitan conflictos.

### Pines libres restantes

- **A0**: ADC (entrada analógica; no usado).
- No hay más GPIOs digitales cómodos sin reasignar MAX/OLED.

## Diagrama lógico

```
ESP8266
  D6/D5 ── I2C ── OLED 0x3C
  D8/D4/D0 ── MAX7219
  D7/D1/D2/D3 ── ST7789 (CS→GND, BLK→3V3)
```
