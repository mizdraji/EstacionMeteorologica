#include "TaskManager.h"
#include "../Config.h"
#include "IndoorData.h"
#include "../network/StationWiFi.h"
#include "../network/MqttSubscriber.h"
#include "../network/NtpTime.h"
#include "../display/Max7219Display.h"
#include "../display/OledDisplay.h"
#include "../ui/LcdUi.h"
#include <TaskScheduler.h>

static Scheduler scheduler;
static StationWiFi stationWiFi;
static Max7219Display max7219;

static unsigned long bootMillis = 0;
static bool displaysReady = false;

static void holdIdlePins();
static void initDisplaysDeferred();
static void taskWiFiCallback();
static void taskNtpCallback();
static void taskMqttCallback();
static void taskClockCallback();
static void taskOledCallback();
static void taskLcdCallback();
static void taskSystemCallback();

static Task taskWiFi(WIFI_INTERVAL_MS, TASK_FOREVER, &taskWiFiCallback);
static Task taskNtp(NTP_TASK_INTERVAL_MS, TASK_FOREVER, &taskNtpCallback);
static Task taskMqtt(MQTT_TASK_INTERVAL_MS, TASK_FOREVER, &taskMqttCallback);
static Task taskClock(CLOCK_INTERVAL_MS, TASK_FOREVER, &taskClockCallback);
static Task taskOled(OLED_INTERVAL_MS, TASK_FOREVER, &taskOledCallback);
static Task taskLcd(LCD_INTERVAL_MS, TASK_FOREVER, &taskLcdCallback);
static Task taskSystem(SYSTEM_INTERVAL_MS, TASK_FOREVER, &taskSystemCallback);

void TaskManager::begin() {
  bootMillis = millis();
  holdIdlePins();
  initDisplaysDeferred();
  // No retocar CS/CLK del MAX tras LedControl (holdIdlePins solo LoRa/LCD RST).
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);

  // LCD vive en HSPI; no hace falta SPI.begin/end del bus global (VSPI).
  if (IndoorData::instance().lcdOK) {
    LcdUi::claimBus();
    LcdUi::invalidate();
  }

  Serial.println(F("[BOOT] WiFi..."));
  stationWiFi.begin();
  NtpTime::begin();
  MqttSubscriber::begin();

  scheduler.addTask(taskWiFi);
  scheduler.addTask(taskNtp);
  scheduler.addTask(taskMqtt);
  scheduler.addTask(taskClock);
  scheduler.addTask(taskOled);
  scheduler.addTask(taskLcd);
  scheduler.addTask(taskSystem);

  taskWiFi.enable();
  taskNtp.enable();
  taskMqtt.enable();
  taskClock.enable();
  taskOled.enable();
  taskLcd.enable();
  taskSystem.enable();

  Serial.println(F("[BOOT] TaskScheduler listo"));
}

void TaskManager::run() {
  scheduler.execute();
}

static void holdIdlePins() {
  // LoRa en reset (RST LOW). GPIO18/5 = MAX CLK/CS (no forzar LoRa CS HIGH).
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);
  pinMode(LCD_RST_PIN, OUTPUT);
  digitalWrite(LCD_RST_PIN, HIGH);
  pinMode(MAX7219_CS_PIN, OUTPUT);
  digitalWrite(MAX7219_CS_PIN, HIGH);
}

static void initDisplaysDeferred() {
  IndoorData& data = IndoorData::instance();

  // LCD primero (HSPI 23/13): luego MAX bitbang en 21/18/5 — buses distintos.
  // NO llamar SPI.end(): destruía el bus del panel y dejaba splash/negro.
#if OLED_ENABLED
  Serial.println(F("[BOOT] OLED..."));
  data.oledOK = OledDisplay::begin();
  if (data.oledOK) {
    OledDisplay::showBoot("Weather", FIRMWARE_VERSION);
  }
#else
  data.oledOK = OledDisplay::begin();  // log + false
#endif

  Serial.println(F("[BOOT] LCD HSPI (bus propio, no LoRa/VSPI)..."));
  data.lcdOK = LcdUi::begin();
  if (data.lcdOK) {
    Serial.println(F("[BOOT] LCD OK → splash boot"));
    LcdUi::showBoot();
  } else {
    Serial.println(F("[BOOT] LCD FAIL (begin=false)"));
  }

  // MAX en bitbang puro — no compartir ni liberar HSPI del LCD.
  delay(20);
  holdIdlePins();

  Serial.println(F("[BOOT] MAX7219 LedControl (post-LCD, init única)..."));
  Serial.flush();
  data.max7219OK = max7219.begin(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_CS_PIN, true);
  if (data.max7219OK) {
    max7219.setIntensity(MAX7219_INTENSITY);
    max7219.showDashes();
    Serial.print(F("[MAX7219] OK LedControl DIN/CLK/CS GPIO"));
    Serial.print(MAX7219_DIN_PIN);
    Serial.print('/');
    Serial.print(MAX7219_CLK_PIN);
    Serial.print('/');
    Serial.println(MAX7219_CS_PIN);
    Serial.println(F("[MAX7219] dashes (sin NTP → --------)"));
  } else {
    Serial.println(F("[MAX7219] skip/fail"));
  }

  // Tras MAX: reclamar HSPI y forzar UI (sale del splash aunque aún no haya MQTT).
  if (data.lcdOK) {
    LcdUi::claimBus();
    LcdUi::invalidate();
    Serial.println(F("[BOOT] LCD claimBus + invalidate post-MAX"));
    LcdUi::render(data);
  }

  displaysReady = true;
  Serial.println(F("[BOOT] displays diferidos OK"));
}

static void taskWiFiCallback() { stationWiFi.update(); }
static void taskNtpCallback() { NtpTime::update(); }
static void taskMqttCallback() { MqttSubscriber::update(); }

static void taskClockCallback() {
  if (!displaysReady || !IndoorData::instance().max7219OK) {
    return;
  }
  // Solo update de dígitos — NUNCA begin/re-init aquí. Bitbang 21/18/5 ≠ HSPI LCD.
  if (NtpTime::isSynced()) {
    max7219.showTime(NtpTime::hours(), NtpTime::minutes(), NtpTime::seconds());
  } else {
    max7219.showDashes();
  }
}

static void taskOledCallback() {
  if (!displaysReady) {
    return;
  }
  if (!IndoorData::instance().oledOK) {
    return;
  }
  OledDisplay::showStatus(IndoorData::instance());
}

static void taskLcdCallback() {
  if (!displaysReady) {
    return;
  }
  if (!IndoorData::instance().lcdOK) {
    return;
  }
  LcdUi::render(IndoorData::instance());
}

static void taskSystemCallback() {
  IndoorData& data = IndoorData::instance();
  data.uptime = (millis() - bootMillis) / 1000UL;
  data.freeHeap = ESP.getFreeHeap();
}
