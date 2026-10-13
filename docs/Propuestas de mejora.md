# Propuestas de mejora — indoor y outdoor

Revisión del monorepo `EstacionMeteorologica` (outdoor ESP8266 fw **1.6.1**, indoor ESP32 TTGO fw **0.3.9**). Cada ítem incluye el problema observado, el fundamento técnico y una propuesta concreta. Las prioridades:

| Prioridad | Criterio |
|-----------|----------|
| **P0** | Fiabilidad, datos incorrectos o riesgo de hang/WDT en el dispositivo |
| **P1** | Robustez operativa (WiFi, MQTT, OTA, heap) con impacto alto |
| **P2** | UX, mantenibilidad y deuda técnica |
| **P3** | Evolución (sensores, HA, TLS) |

---

## 1. Contrato MQTT: LWT, birth y mensaje retained

**Prioridad:** P0  
**Afecta:** outdoor `MqttPublisher.cpp` + indoor `MqttSubscriber.cpp` / `IndoorData`

**Qué hay hoy.** Outdoor publica QoS 0, `retain=false`, sin Last Will. Indoor solo infiere “outdoor vivo” con `MQTT_STALE_MS` (30 s) sobre el último JSON.

**Fundamento.** En MQTT el suscriptor no distingue *broker caído*, *WiFi del outdoor caído* y *outdoor colgado*. Un LWT (`offline`) lo publica el broker cuando se pierde el TCP keep-alive; un birth (`online`) al conectar cierra el ciclo. Con `retain` en el último telemetría, el indoor muestra datos al suscribirse en lugar de esperar hasta 30 s el próximo publish. PubSubClient soporta `connect(..., willTopic, willQoS, willRetain, willMessage)`.

**Propuesta.** Topic de estado `…/status` (o campo `status` retained) con `online`/`offline`. Telemetría retained en el topic actual *o* en `…/telemetry`. Indoor: si LWT=`offline`, mostrar “Outdoor off”; si no llega telemetría, “MQTT stale”. Keep-alive 30 s ya está; alinear LWT a 1.5–2× keep-alive.

---

## 2. `MQTT_STALE_MS` igual al intervalo de publicación

**Prioridad:** P0  
**Afecta:** outdoor `MQTT_INTERVAL_MS = 30000` · indoor `MQTT_STALE_MS = 30000`

**Qué hay hoy.** Outdoor publica cada 30 s. Indoor marca datos viejos a los 30 s exactos y pinta `-` en temperatura/humedad/presión/OWM.

**Fundamento.** Un jitter de WiFi, un `publish` fallido o un `loop()` retrasado por I2C/HTTP hace que `millis() - lastMqttMs` cruce 30 s aunque el nodo exterior esté sano. La UI entra en “Sin MQTT” en falso. La regla habitual es **2.5×–3×** el periodo de publicación (más un margen de keep-alive).

**Propuesta.** `MQTT_STALE_MS` ≥ `90000` (o derivarlo: `3 * MQTT_INTERVAL_MS`). Documentar que “stale” ≠ “nunca hubo datos” (`mqttHasData`).

---

## 3. Buffer MQTT outdoor demasiado justo (384 B)

**Prioridad:** P0  
**Afecta:** `outdoor/src/network/MqttPublisher.cpp` (`StaticJsonDocument<384>`, `char payload[384]`, `setBufferSize(384)`)  
Indoor ya usa 512 y `-D MQTT_MAX_PACKET_SIZE=512`. Outdoor **no** define ese flag.

**Qué hay hoy.** El JSON incluye `temp_*`, flags, `ext_desc` (hasta 47 caracteres), IP, firmware, heap, RSSI. Con descripción OWM larga (`"nubes dispersas con tormentas"`) `serializeJson` puede devolver `n >= 384`, se loguea `publish fail (payload)` y **no sale telemetría**.

**Fundamento.** ArduinoJson cuenta bytes del documento serializado, no del `StaticJsonDocument` interno. Un corte silencioso en el publicador deja al indoor en stale. PubSubClient 2.8 respeta `setBufferSize`, pero el paquete MQTT = topic + payload + cabecera; 384 para el JSON deja poco margen.

**Propuesta.** Subir a **512** (igual que indoor), añadir `-D MQTT_MAX_PACKET_SIZE=512` en `outdoor/platformio.ini`, y si `n` se acerca al tope, recortar `ext_desc` en lugar de abortar todo el publish.

---

## 4. `mqttClient.connect()` bloqueante vs WDT del loop (indoor)

**Prioridad:** P0  
**Afecta:** indoor `main.cpp` (`enableLoopWDT()`) + `MqttSubscriber::tryConnect()`

**Qué hay hoy.** El loop WDT está activo. `PubSubClient::connect()` es TCP bloqueante (DNS + SYN + CONNACK) y puede durar varios segundos si el broker no responde.

**Fundamento.** En ESP32 Arduino el WDT del task `loop` suele ser ~5 s. Un connect colgado o lento dispara `ESP_RST_TASK_WDT` / panic (`Reset: 4` en el boot, ya documentado en el README indoor). Outdoor no habilita ese WDT, por eso “falla peor” el indoor.

**Propuesta (en orden de costo).** (1) `disableLoopWDT()` alrededor del `connect` y reactivar después; (2) timeout corto en `WiFiClient::setTimeout`; (3) a medio plazo, MQTT asíncrono (`espMqttClient` / `AsyncMqttClient`) en una task FreeRTOS. No subir el WDT “para siempre”: tapa el síntoma.

---

## 5. Lectura AHT10 con `delay(80)` en el hilo del scheduler

**Prioridad:** P0  
**Afecta:** `outdoor/src/sensors/AHT10Sensor.cpp` (`readRaw`: trigger + `delay(80)` + reintentos `delay(10)`)

**Qué hay hoy.** Cada 2 s (`AHT_INTERVAL_MS`) la tarea AHT bloquea ~80–130 ms. En ese tiempo **no** corren `taskWeb` (50 ms) ni `taskOTA` (30 ms).

**Fundamento.** TaskScheduler es cooperativo: un callback largo es un freeze del resto. ArduinoOTA necesita `handle()` frecuente; un bloqueo de 100 ms repetido + HTTP OWM de 8 s (`http.setTimeout(8000)`) es la causa clásica de OTA a medias y web que “no responde”. En ESP8266 `delay()` hace `yield()`, así que el WDT de software no muerde, pero **OTA y TCP sí se degradan**.

**Propuesta.** Máquina de estados: `trigger` → return; 80 ms después (tick del scheduler, no `delay`) → `requestFrom`. Mismo patrón que un ADC asíncrono. Durante `otaInProgress`, no lanzar AHT/BMP/OWM.

---

## 6. BMP180: delays + lecturas duplicadas por sample

**Prioridad:** P1  
**Afecta:** `BMP180Sensor.cpp` + `BMP085.cpp`

**Qué hay hoy.** Un `read()` hace `readTemperature()` (delay 5 ms), `readPressure()` (otra temperatura 5 ms + presión 8 ms en STANDARD) y `readAltitude()` (típicamente **otra** presión). Son ~30 ms bloqueantes y conversiones I2C repetidas cada 1 s.

**Fundamento.** El BMP180 no guarda un “último UT/UP” entre APIs de Adafruit: cada método dispara su propia conversión. La altitud además usa `SEA_LEVEL_PRESSURE_HPA = 1013.25`, que **no** es el QNH local: el número en metros es cosmética, no metrología.

**Propuesta.** Una sola pareja UT/UP por ciclo; derivar T, P y altitud en software. Bajar `BMP180_INTERVAL_MS` a 2–3 s (el chip no aporta más). QNH editable en `Config.h` o en la web (`/api/status`). Documentar que la altitud es estimada.

---

## 7. `Wire.begin()` en cada lectura I2C

**Prioridad:** P1  
**Afecta:** `I2CBus::beginBMP()` llamado desde `AHT10Sensor::read` y `BMP180Sensor::read`

**Qué hay hoy.** Cada sample re-inicializa el bus a 100 kHz.

**Fundamento.** `Wire.begin()` en ESP8266 reconfigura pines y el controller I2C. En un bus compartido (AHT10 0x38 + BMP180 0x77) eso puede generar glitches START/STOP y lecturas `aht10_ok`/`bmp180_ok` intermitentes. 100 kHz es correcto (cables largos al exterior); reinit no lo es.

**Propuesta.** `I2CBus::beginBMP()` solo en `SensorManager::begin()`. Opcional: un `I2CBus::scan()` al boot en el banner serie (direcciones 0x38 / 0x77) para diagnosticar cableado sin adivinar.

---

## 8. Portal WiFi outdoor bloqueante (hasta 180 s)

**Prioridad:** P1  
**Afecta:** `outdoor/src/network/StationWiFi.cpp` (`wm.autoConnect` con portal bloqueante) vs indoor (`setConfigPortalBlocking(false)`)

**Qué hay hoy.** `TaskManager::begin()` llama `stationWiFi.begin()` **antes** de `enable()` de las tareas. Si no hay AP o la clave cambió, el nodo se queda hasta `WIFI_CONFIG_PORTAL_TIMEOUT_SEC` (180 s) **sin** leer sensores, sin MQTT y sin web.

**Fundamento.** Indoor ya resolvió esto con portal no bloqueante + `_wm.process()` en el loop. Outdoor es el que más necesita seguir midiendo con WiFi caído (estación exterior). El patrón STA + reintento `WiFi.begin()` cada 5 s está bien *después* del primer connect; el primer boot es el agujero.

**Propuesta.** Mismo modelo que indoor: `setConfigPortalBlocking(false)`, `process()` en `taskWiFi`, sensores habilitados igual. Al conectar, iniciar mDNS/web/OTA (eso ya está en `update()`).

---

## 9. Indoor deja el radio en `WIFI_AP_STA`

**Prioridad:** P1  
**Afecta:** `indoor/src/network/StationWiFi.cpp`

**Qué hay hoy.** `WiFi.mode(WIFI_AP_STA)` al inicio y otra vez si reabre el portal. Tras un connect exitoso **no** pasa a `WIFI_STA`.

**Fundamento.** En AP_STA el ESP32 sigue anunciando el SoftAP `weather-indoor-01`. Es superficie de ataque en LAN (portal de config) y consume heap/radio que el LCD/MQTT necesitan. WiFiManager, si el portal no está activo, no implica que el AP se apague si el modo sigue siendo AP_STA.

**Propuesta.** Si `WL_CONNECTED` y portal inactivo → `WiFi.mode(WIFI_STA)`. Reabrir AP solo en el fallback offline (el código ya lo hace).

---

## 10. OWM: un fallo HTTP borra el último clima bueno

**Prioridad:** P1  
**Afecta:** `ExternalWeather.cpp` (`markFail`) + indoor vista OWM (`extOK && extDesc[0]`)

**Qué hay hoy.** HTTP ≠ 200 o JSON incompleto pone `externalOK=false` y anula temp/hum/desc. Indoor entonces **abandona** la vista “CONDICION OWM” y muestra WiFi/MQTT.

**Fundamento.** El free tier y el DNS fallan de forma transitoria. Tirar el último current weather válido hace que la UI mienta (“no hay OWM”) cuando sí hubo un valor de hace 8 minutos. Ya existe `externalLastUpdateMs` y la API web expone `external_age_s`; el publicador MQTT **no** manda esa edad.

**Propuesta.** En error: conservar última muestra, `ext_ok=false` o `ext_stale=true`, publicar `ext_age_s`. Indoor: seguir mostrando la descripción con etiqueta “hace Nm”. Solo borrar si nunca hubo éxito. Backoff 2× si HTTP 429/5xx (el intervalo fijo de 10 min es correcto para el free tier, no para un 401).

---

## 11. OpenWeatherMap por HTTP en claro y con `String`

**Prioridad:** P1 (seguridad de la API key) / P2 (heap)  
**Afecta:** `buildUrl` / `fetchOnce` en `ExternalWeather.cpp`

**Qué hay hoy.** `http://api.openweathermap.org/...&appid=KEY`. Payload en `String`. Lat/lon con 14 decimales.

**Fundamento.** La API key viaja en claro (LAN y hacia Internet). En ESP8266, BearSSL cabe si se acota el heap (ya hay filtro Json de 192+384). `String` en el 8266 fragmenta el heap de ~30–40 KB libres típicos con web+MQTT. 14 decimales de float no añaden precisión (el `float` de `OWM_LAT` ya recorta).

**Propuesta.** URL en `char[]` (~160 B), lat/lon a 4–5 decimales. Valorar HTTPS (`WiFiClientSecure` + fingerprint/cert) o, si el heap no da, documentar el riesgo y restringir la key por IP/dominio en la cuenta OWM. No interpolar la key en logs.

---

## 12. Epoch NTP “local” (NTPClient + offset)

**Prioridad:** P1  
**Afecta:** `NtpTime` indoor/outdoor · `HistoryBuffer` · `script.js` (`getUTCHours` *adrede*)

**Qué hay hoy.** `NTPClient(..., NTP_TIME_OFFSET_SEC)` hace que `getEpochTime()` **no** sea Unix UTC. La web corrige con getters UTC. Indoor usa horas/minutos para el MAX, coherente con el offset.

**Fundamento.** Cualquier consumidor (Home Assistant, Grafana, un segundo indoor) interpretará `epoch` como UTC y desplazará los gráficos 3 h. El comentario en JS es un parche frágil: si alguien “arregla” NTP a UTC real, los ejes se rompen al revés. Argentina (ART) es UTC−3 todo el año desde 2009; igual conviene TZ POSIX (`ART3`) + `configTime()` / `time()` del SDK.

**Propuesta.** Guardar **UTC real** en historial y MQTT (`ts_utc`). Derivar hora local solo para UI. Unificar indoor y outdoor en el mismo helper (hoy hay dos `NtpTime.cpp` casi idénticos). Servidor NTP: lista de fallback, no un único `3.south-america.pool.ntp.org`.

---

## 13. LCD: `fillScreen` cada ~2 s por heap/uptime

**Prioridad:** P1  
**Afecta:** `LcdUi::render` (condiciones `uptime` / `freeHeap`) + `drawView` → `tft.fillScreen(COL_BG)`

**Qué hay hoy.** `taskSystem` actualiza uptime y heap cada 1 s. Eso marca `changed=true` en **todas** las vistas. El throttle `LCD_INTERVAL_MS` (2 s) limita, pero cada update hace wipe/fill de 240×240.

**Fundamento.** Un frame ST7789 es 240×240×2 ≈ 115 KB por SPI. A ~20 MHz son decenas de ms; con wipe al rotar, el doble. El heap de ESP32 **oscila** por WiFi; casi nunca es igual al frame anterior. Resultado: parpadeo, bus ocupado (peor convivencia con MAX bitbang) y logs `[LCD] draw vals` cada 2 s.

**Propuesta.**  
- No incluir heap/uptime en `changed` salvo `_view == sistema`.  
- Redibujar solo el rectángulo del número, no `fillScreen`, si la vista no cambió.  
- Chrome (reloj HH:MM): invalidar solo la franja superior.  
- Bajar verbosidad serie (`logRenderDecision` solo si `DEBUG_LCD`).

---

## 14. OTA en indoor (hoy no existe)

**Prioridad:** P1  
**Afecta:** indoor (plantilla `OTA_PASSWORD` comentada en `secrets.h.example`) vs outdoor `OTAManager.cpp`

**Qué hay hoy.** El display de pared exige USB + `pio run -t upload`. Outdoor ya tiene ArduinoOTA con password.

**Fundamento.** El indoor es el nodo menos accesible físicamente. El stack (WiFi + MQTT + GFX) es el mismo patrón que outdoor; ESP32 tiene más heap que el 8266, así que OTA es más viable aquí que allí. Sin OTA, cada 0.3.x implica desmontar.

**Propuesta.** Portar `OTAManager` al ESP32 (`ArduinoOTA` + hostname `weather-indoor-01`). En `onStart`, pausar LCD/MAX (o dejar el último frame) para no mezclar SPI/bitbang con el flash. Password en `secrets.h`, nunca vacía.

---

## 15. Shutdown incompleto de web y OTA (outdoor)

**Prioridad:** P1  
**Afecta:** `WebServerManager::shutdown` (`_webReady = false`) · `OTAManager::shutdown` (`_otaReady = false`)

**Qué hay hoy.** Al perder WiFi se deja de llamar `handleClient` / `ArduinoOTA.handle()`, pero **no** hay `server.stop()` ni se destruye el objeto OTA. `begin()` posterior hace early-return si el flag ya se puso true en un ciclo anterior… OTA: si `_otaReady` queda false, el próximo `begin()` sí reinicia. Web: `_webReady=false` permite `begin()` de nuevo. El servidor HTTP puede quedar a medias con sockets abiertos.

**Fundamento.** En ESP8266, no parar el `ESP8266WebServer` deja PCBs en `TIME_WAIT` y heap retenido. Tras un roam de AP, `/api/data` a veces “muere” hasta reset.

**Propuesta.** `server.stop()`, `MDNS.end()`, y un `ArduinoOTA.end()` (o no reusar el flag como único estado). Idempotente: `begin()` siempre debe poder rearmar.

---

## 16. Historial solo en RAM (~30 min) y se pierde al reset

**Prioridad:** P2  
**Afecta:** `HistoryBuffer` (`HISTORY_CAPACITY 60` × `HISTORY_INTERVAL_MS 30000`)

**Qué hay hoy.** La web lo documenta: se pierde al reiniciar. No hay append a LittleFS.

**Fundamento.** Un reset por WDT/OTA borra la única serie temporal local. LittleFS ya está montado para la UI. 60 muestras × 16 B ≈ 1 KB: se puede circular en RAM **y** flushear cada N puntos a `/history.bin` o CSV.

**Propuesta.** Persistir ring buffer cada 5 min; al boot, hidratar. Opcional: subir capacidad a 120 (1 h) si el heap lo permite. No usar `String` para serializar el archivo.

---

## 17. Schema MQTT ≠ schema HTTP

**Prioridad:** P2  
**Afecta:** `MqttPublisher` (`temp_main`, `ext_temp`) vs `handleApiData` (`temperature_main`, `external_temperature`)

**Qué hay hoy.** Dos vocabularios para el mismo estado. Indoor parsea el MQTT; la web el HTTP. `bmp180_ok`/`aht10_ok` llegan por MQTT pero indoor los marca “reservado” y no los pinta.

**Fundamento.** Dos parsers = dos bugs. Home Assistant y Node-RED copiarán uno u otro y se romperán al cambiar un campo. No hay `schema` / `fw` / `ts` en el JSON MQTT (sí hay `firmware`).

**Propuesta.** Un único documento de contrato (`docs/mqtt.md` como fuente) con `v: 1`. Añadir `ts_utc`, `ext_age_s`, `rssi`. Indoor vista Sistema: IP outdoor, fw outdoor, `aht10_ok`/`bmp180_ok` (ya se copian a `IndoorData`). Alinear nombres o publicar **el mismo** JSON en `/api/data`.

---

## 18. Filtro de sensores (picos y hysteresis)

**Prioridad:** P2  
**Afecta:** `AHT10Sensor::read` / `BMP180Sensor::read`

**Qué hay hoy.** Rango duro (T −40…85, HR 0…100, P 300…1100). Un spike de un sample pasa a MQTT y al LCD. Un fallo pone `*_ok = false` y `temperatureMain` salta al BMP (o a inválido) **en el mismo ciclo**.

**Fundamento.** I2C ruidoso en exterior (humedad, cables) produce un 0xFF ocasional que el rango no siempre pesca. Cambiar de AHT a BMP como `temp_main` sin avisar genera un diente de sierra de 0.5–2 °C en la gráfica.

**Propuesta.** Mediana de 3 (o EMA α≈0.3). Exigir N fallos consecutivos antes de `aht10OK=false`. Publicar `temp_src` (`aht`|`bmp`) para que indoor/web lo muestren.

---

## 19. `LcdUi::begin()` siempre retorna `true`

**Prioridad:** P2  
**Afecta:** `LcdUi.cpp` / `TaskManager::initDisplaysDeferred`

**Qué hay hoy.** Tras `tft.init` no hay probe (el GMT130 no tiene CS ni MISO). `lcdOK` queda true aunque el panel esté desconectado; el código sigue haciendo `fillScreen` sobre un bus vacío.

**Fundamento.** No se puede leer el ID del ST7789 sin MISO. Igual se puede tratar el LCD como “best effort” y no bloquear el boot del MAX/MQTT si se añade un flag de config `LCD_ENABLED`. Lo importante es no mentir en logs `[BOOT] LCD OK`.

**Propuesta.** Documentar “init fire-and-forget”. Timeout: si tras N frames nadie “ve” el splash, no es detectable en software; el env `indoor_lcd_test` ya cubre hardware. Evitar `holdIdlePins()` **después** de que LedControl posee CS/CLK (hoy se llama otra vez en `initDisplaysDeferred` y puede picar el MAX).

---

## 20. Redundancia de `holdIdlePins` y conflicto LoRa/MAX

**Prioridad:** P2 (estabilidad de displays)  
**Afecta:** `indoor/src/main.cpp` + `TaskManager.cpp` (la misma función, 3 llamadas en boot)

**Qué hay hoy.** LoRa RST=14 LOW está bien documentado. GPIO18/5 son CLK/CS del MAX **y** CS/SCK del SX127x. Cualquier `pinMode(MAX7219_CS)` extra tras `LedControl` puede dejar un dígito fantasma.

**Fundamento.** El propio código avisa: “No retocar CS/CLK del MAX tras LedControl”. `main` y `TaskManager` duplican la secuencia. Es deuda de debug (0.3.x) que ya no aporta.

**Propuesta.** Una sola función `BoardPins::holdLoRaReset()` en boot, **sin** tocar 18/5 una vez el MAX hizo `begin`. Mantener RST LoRa LOW en `taskSystem` por si el pin flota (opcional, 1 Hz).

---

## 21. MQTT y OWM en claro; API web sin auth

**Prioridad:** P2 (contexto LAN doméstica)  
**Afecta:** `MQTT_PORT 1883` ambos nodos · `ESP8266WebServer` en puerto 80

**Qué hay hoy.** Usuario/password MQTT viajan en CONNECT sin TLS. `/api/data` es GET público en la LAN. OTA outdoor sí tiene password.

**Fundamento.** En casa suele ser aceptable. En WiFi de invitados o IoT VLAN mal segmentada, cualquiera publica al topic (el indoor lo creerá) o flashea si adivina OTA. TLS en 8266 es caro; en ESP32 indoor es razonable (`WiFiClientSecure` + puerto 8883).

**Propuesta.** Corto plazo: broker solo en LAN, user/pass no vacíos en producción, OTA fuerte. Medio plazo: MQTTS en indoor; outdoor permanece 1883 o se mueve a un broker local. Web: token en header o desactivar API si no hay sesión (el 8266 no da para HTTPS cómodo).

---

## 22. Configuración MQTT/OWM sin recompilar

**Prioridad:** P2  
**Afecta:** `secrets.h` (gitignore correcto) + WiFiManager solo para SSID

**Qué hay hoy.** Cambiar broker o API key implica editar `secrets.h`, compilar y USB/OTA. WiFi sí es portal.

**Fundamento.** WiFiManager soporta `WiFiManagerParameter`. ESP32 tiene NVS; ESP8266 puede guardar en LittleFS (`/config.json`). El indoor ya pide “mismos valores que outdoor” a mano: es la fuente nº 1 de “no llega MQTT”.

**Propuesta.** Campos en el portal: host, puerto, user, pass, topic. Validar en boot; si faltan, portal obligatorio. `secrets.h` queda como default de fábrica / override de compile.

---

## 23. Vista Sistema indoor: no usa datos outdoor ya parseados

**Prioridad:** P2  
**Afecta:** `IndoorData` (`outdoorIp`, `outdoorFirmware`, `bmp180OK`, `aht10OK`) vs `drawSysView` (solo uptime/heap **local**)

**Qué hay hoy.** El JSON se parsea y se descarta en UI. Serial `[SYS]` tampoco muestra salud del exterior.

**Fundamento.** El indoor es el monitor humano del sistema. Heap local &lt; 48 KB está bien; no dice si el AHT10 del techo falló. Es el cambio de UX con mejor ratio esfuerzo/valor tras el stale window.

**Propuesta.** Vista Sistema (o una 6ª vista): fw indoor/outdoor, IP outdoor, flags sensores, RSSI indoor, opcional RSSI outdoor (`rssi` ya va en el payload). Heap warn se queda.

---

## 24. Brillo MAX7219 fijo e intensidad LCD

**Prioridad:** P3  
**Afecta:** `MAX7219_INTENSITY 2` · BLK del ST7789 atado a 3V3

**Fundamento.** Intensidad 2 es razonable de noche y puede quedar corta de día. BLK a 3V3 impide PWM de backlight. Con NTP ya hay hora local: se puede bajar a 0–1 entre 23:00–07:00 y subir a 4–6 de día, sin sensor de luz.

**Propuesta.** `MAX7219_NIGHT_INTENSITY` / `DAY` por hora. Si se recablea BLK a un GPIO con PWM (p. ej. GPIO25 LED LilyGO), atenuar LCD igual.

---

## 25. Botón para fijar / avanzar vista LCD

**Prioridad:** P3  
**Afecta:** UI indoor; hay GPIOs libres (evitar strapping 0/2/12/15)

**Fundamento.** Rotación 7 s es buena para un vistazo y mala para leer presión o heap. Un botón a GND (interno pull-up) es el control más barato.

**Propuesta.** Click: siguiente vista y reiniciar `LCD_VIEW_ROTATE_MS`. Long-press: freeze. No usar GPIO34–39 sin pull-up externo (input-only).

---

## 26. Versiones de platform/libs sin pin y docs desfasadas

**Prioridad:** P2  
**Afecta:** ambos `platformio.ini` (`platform = espressif32` / `espressif8266` sin versión) · `outdoor/README.md` cita indoor **0.3.7** (hoy **0.3.9**)

**Fundamento.** Un `pio pkg update` puede romper WiFiManager, el core 8266 y Adafruit ST7789 (el MODE3 + CS=-1 es sensible). El README outdoor desactualizado hace dudar qué fw está en la placa.

**Propuesta.** Pin: `platform = espressif32@6.x` / `espressif8266@4.x` y `lib_deps` con versión exacta (ya hay `^`, mejor `==` en producción). CI: dos jobs `pio run` en `outdoor/` e `indoor/` (envs `ttgo` + `nodemcuv2`). Corregir 0.3.7 → 0.3.9. Un `FIRMWARE_VERSION` por nodo está bien; el contrato MQTT debe llevarlo siempre.

---

## 27. Duplicación NtpTime / JSON float / backoff MQTT

**Prioridad:** P2  
**Afecta:** copias indoor vs outdoor

**Fundamento.** El backoff 1 s → 60 s está bien (y repetido). `setJsonFloatOrNull` / `jsonFloatOrInvalid` son el mismo contrato. Dos plataformas (Arduino-ESP32 vs ESP8266) impiden un `.a` único, pero se puede `shared/` con `#ifdef ESP32` o un submódulo de headers.

**Propuesta.** No es urgente. Cuando se toque NTP (ítem 12), extraer `NtpTime` y el helper JSON. Evitar un tercer copy-paste si aparece un nodo “taller”.

---

## 28. Sensores: camino de evolución (AHT20 / BMP280)

**Prioridad:** P3  
**Afecta:** outdoor I2C

**Fundamento.** AHT10 no trae CRC; AHT20 sí (byte de checksum). BMP180 está discontinuado; BMP280/BME280 miden T+P(+H) con menos delays y driver Adafruit no bloqueante. El pinout actual (D1/D2) se reutiliza.

**Propuesta.** No cambiar hardware ahora. Si hay recambio: AHT20 en 0x38 (drop-in) + CRC en `readRaw`. Un BME280 puede retirar el BMP180 y simplificar `temp_main`.

---

## 29. Home Assistant / descubrimiento MQTT

**Prioridad:** P3

**Fundamento.** El JSON plano funciona con `mosquitto_sub`. HA espera `homeassistant/sensor/…/config` (MQTT Discovery) o un dispositivo MQTT con entidades T/H/P.

**Propuesta.** Tras LWT + schema v1, publicar discovery retained una vez por boot. No hace falta cambiar el topic de telemetría.

---

## 30. Pausar trabajo pesado durante OTA (outdoor)

**Prioridad:** P1 (complementa el ítem 5)

**Qué hay hoy.** `otaInProgress` se setea pero **ninguna** tarea lo consulta. OWM puede hacer GET de 8 s en medio de un flash.

**Fundamento.** Flash OTA + HTTPClient + I2C delay es la receta de `Fatal exception` en 8266.

**Propuesta.** Si `otaInProgress`: return inmediato en AHT, BMP, OWM, MQTT publish e historial. Seguir con `taskOTA` + `taskWeb` + WiFi.

---

## Orden de implementación sugerido

1. Stale 90 s + buffer MQTT 512 + LWT (ítems 1–3) — un día, máximo impacto en la UI indoor.  
2. No bloquear AHT/OTA y respetar `otaInProgress` (5, 6, 30).  
3. Portal outdoor no bloqueante + indoor `WIFI_STA` (8, 9).  
4. WDT vs `connect` y OTA indoor (4, 14).  
5. LCD parcial + vista salud outdoor (13, 23).  
6. OWM conservador + NTP UTC (10, 12).  
7. Pin de plataformas, CI y README (26).

---

## Lo que está bien (no tocar sin motivo)

- Separación indoor/outdoor y `secrets.h` en `.gitignore`.  
- WiFiManager para SSID (no hardcodear WiFi).  
- Sentinel `WEATHER_VALUE_INVALID` (−9999) en lugar de usar 0.  
- HSPI dedicado al ST7789 y bitbang LedControl en 21/18/5 (el conflicto LoRa está **documentado y mitigado** con RST=14 LOW).  
- Envs de diagnóstico `indoor_minimal` / `indoor_lcd_test` / `indoor_max_test`.  
- Filtro ArduinoJson en OWM (heap 8266).  
- `dataIsFresh()` con resta unsigned de `millis()` (overflow-safe).  
- Payload MQTT con `null` en lecturas inválidas.
