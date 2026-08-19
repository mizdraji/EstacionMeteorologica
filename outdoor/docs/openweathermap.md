# OpenWeatherMap (clima externo)

El ESP8266 consulta la API **Current Weather** de OpenWeatherMap y expone temp/humedad/descripción en `/api/data`. El browser **no** llama a OpenWeatherMap (evita CORS y no expone la API key).

## API key (free)

1. Cuenta en [openweathermap.org/api](https://openweathermap.org/api)
2. Suscripción **Current Weather Data** (plan Free)
3. En *API keys*, copiá la key (puede tardar unos minutos en activarse)
4. En `src/secrets.h`:

```cpp
#define OPENWEATHERMAP_API_KEY    "tu_key_aqui"
```

Plantilla: `src/secrets.h.example`. `secrets.h` no se versiona.

## Ubicación

En `src/Config.h`:

| Define | Uso |
|--------|-----|
| `OWM_USE_CITY_ID` | `1` = city id; `0` = lat/lon |
| `OWM_CITY_ID` | Ej. Buenos Aires `3435217` |
| `OWM_LAT` / `OWM_LON` | Coordenadas si `OWM_USE_CITY_ID` es `0` |
| `OWM_INTERVAL_MS` | Intervalo de consulta (default 10 min) |

Units: `metric`. Idioma: `es`.

## Subida

Hay que actualizar **firmware y LittleFS** (tarjeta web en `data/`):

```powershell
pio run -t upload
pio run -t uploadfs
```

## Limitaciones (free tier)

- Cuota diaria limitada; no consultar cada pocos segundos.
- Key vacía, sin Wi-Fi o error HTTP → web muestra N/A; sensores locales siguen.
- HTTP (no HTTPS) hacia `api.openweathermap.org` para ahorrar heap en ESP8266.
