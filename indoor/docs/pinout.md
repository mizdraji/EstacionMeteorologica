# Indoor — pinout (ESP32 TTGO LoRa32)

Placa base: **LilyGO TTGO LoRa32 V1** (`board = ttgo-lora32-v1`, env default **`ttgo`**). Firmware indoor **0.3.7**.  
OLED removido; LoRa **no se usa** (RST=GPIO14 LOW; no forzar CS HIGH). Pines en `src/Config.h`.

Referencia pinout por revisión: [espboards.dev TTGO LoRa32](https://www.espboards.dev/esp32/ttgo-lora32/).

## Board PlatformIO

| Env | Board | Notas |
|-----|-------|-------|
| **`ttgo`** (default) | `ttgo-lora32-v1` | OLED hist. SDA=4 / SCL=15 / RST=16; LoRa RST=14 — coincide con pinout V1 |
| Alternativa | `ttgo-lora32-v21new` | V2.1.6: OLED I2C **21/22**, LoRa RST=**23**, SD 13/15/2/14 |
| Alternativa | `esp32dev` | Genérico; mismos GPIOs si cableás igual |

Si el silkscreen / PCB es V2.x, cambiá `board =` en **todos** los envs de `platformio.ini`. MAX usa 21/18/5 (validados); LCD SCK=23 choca con LoRa RST solo en V2+ (aquí LoRa RST=14).

## GPIOs reservados (doc / variantes)

| Grupo | V1.0 | V1.2+ / V2.x | Evitar para MAX |
|-------|------|--------------|-----------------|
| OLED I2C | SDA=4 SCL=15 RST=16 | SDA=**21** SCL=**22** | 21/22 en V2 (pull-ups residuales) |
| LoRa SPI | SCK=5 MISO=19 MOSI=27 CS=18 | igual | 5/18/19/27 |
| LoRa RST / IRQ | RST=14 IRQ=26 | RST=**23** IRQ=26 | 14/26 (y 23 en V2) |
| SD (V1.6+) | — | CS=13 MOSI=15 MISO=2 SCLK=14 | 13/15/2/14 si hay slot |
| Flash | 6–11 | 6–11 | nunca |
| Input-only | 34–39 | 34–39 | no OUTPUT |
| Strapping | 0/2/12/15 | igual | boot sensible |
| LED LilyGO | GPIO25 | GPIO25 | usable OUTPUT (CS) |
| USB UART | 1 / 3 | 1 / 3 | Serial |

V1 usa **ESP32-WROOM** → GPIO17 suele existir; en WROVER/PICO-D4 con PSRAM, 16/17 no están libres. Preferimos no usarlo.

## LCD IPS 240×240 ST7789 / GMT130-V1.0 (HW SPI propio)

Módulo: **GND VCC SCK SDA RES DC BLK** (sin CS). Bus **separado** del LoRa (no usar 5/18/19/27).

| Señal módulo | GPIO | Notas |
|--------------|------|-------|
| CS | — | N/A (`TFT_CS = -1`) |
| DC | **4** | OLED SDA liberado (V1) |
| RST (RES) | **16** | OLED RST liberado (V1) |
| SCK | **23** | HW SPI (no LoRa SCK=5) |
| MOSI (SDA) | **13** | HW SPI (no LoRa MOSI=27) |
| VCC | — | **3V3** |
| BLK | — | **3V3** backlight |
| GND | — | GND |

```cpp
SPIClass lcdSpi(HSPI);                     // NO usar SPI/VSPI (defaults 18/5 = MAX)
lcdSpi.begin(23, -1, 13, -1);              // sck, miso, mosi, ss
Adafruit_ST7789 tft(&lcdSpi, -1, 4, 16);   // CS, DC, RST
tft.init(240, 240, SPI_MODE3);
tft.setRotation(0);
```

`SPI_MODE3` = literatura GMT130 (Adafruit #197).  
**Importante:** VSPI por defecto usa SCK=18 / SS=5 — mismos pines del MAX LedControl. El firmware usa **HSPI** para el LCD y **no** llama `SPI.end()` tras el splash.

## MAX7219 (8 dígitos, hora) — LedControl

Librería **LedControl** (`lib/LedControl`): `LedControl(dataPin, clkPin, csPin, 1)`.
Pines **validados en hardware** por el usuario:

| Señal | GPIO | LedControl arg |
|-------|------|----------------|
| DIN | **21** | dataPin |
| CLK | **18** | clkPin |
| CS | **5** | csPin |

CS idle **HIGH**. GPIO18/5 coinciden con LoRa CS/SCK en la PCB; el SX127x se mantiene con **RST=14 LOW**.

## LoRa SX127x (NO usar)

| Señal | GPIO (V1) |
|-------|-----------|
| SCK | 5 (= MAX CS) |
| MISO | 19 |
| MOSI | 27 |
| CS | 18 (= MAX CLK) |
| RST | **14** → firmware lo deja **LOW** |
| IRQ | 26 |

**Importante:** no forzar CS LoRa (18) en HIGH: ese pin es el **CLK del MAX**.

## OLED (removido)

Histórico V1 (ahora libres / reutilizados):

| Señal | GPIO | Uso actual |
|-------|------|------------|
| SDA | 4 | LCD DC |
| SCL | 15 | libre (strapping: dejar HIGH en boot) |
| RST | 16 | LCD RST |

Código: `OLED_ENABLED=0` en `Config.h`.

## WiFi / otros

| Función | GPIO / nota |
|---------|-------------|
| LED onboard | GPIO25 |
| GPIO21 | MAX DIN (también OLED SDA en V1.2+) |

## Diagnóstico solo MAX (`indoor_max_test`)

```powershell
cd indoor
pio run -e indoor_max_test
pio run -e indoor_max_test -t upload
pio device monitor -e indoor_max_test
```

Esperado: ciclo `00000000`…`99999999` → `88-88-88` → clear → dashes.

## Diagnóstico solo LCD (`indoor_lcd_test`)

```powershell
cd indoor
pio run -e indoor_lcd_test
pio run -e indoor_lcd_test -t erase
pio run -e indoor_lcd_test -t upload
pio device monitor -e indoor_lcd_test
```

Esperado: fills rojo → verde → azul → texto **HELLO**.

## Diagrama lógico

```
ESP32 TTGO LoRa32
  GPIO23/13 ── HSPI ── ST7789 SCK/MOSI   (bus propio ≠ VSPI/MAX)
  GPIO4/16  ── ST7789 DC/RST
  GPIO21/18/5 ── MAX7219 DIN/CLK/CS (LedControl)
  GPIO14    ── LoRa RST = LOW (radio idle)
  3V3 ── ST7789 VCC + BLK + MAX7219 VCC
  GND ── común
```
