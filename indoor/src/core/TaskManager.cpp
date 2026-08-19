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

static void holdBootSafePins();
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
  holdBootSafePins();
  initDisplaysDeferred();
  holdBootSafePins();

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

static void holdBootSafePins() {
  pinMode(MAX7219_DIN_PIN, OUTPUT);
  pinMode(MAX7219_CLK_PIN, OUTPUT);
  digitalWrite(MAX7219_DIN_PIN, LOW);
  digitalWrite(MAX7219_CLK_PIN, HIGH);
  pinMode(LCD_RST_PIN, INPUT);
}

static void initDisplaysDeferred() {
  IndoorData& data = IndoorData::instance();

  Serial.println(F("[BOOT] MAX7219..."));
  data.max7219OK = max7219.begin(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_CS_PIN);
  if (data.max7219OK) {
    max7219.setIntensity(MAX7219_INTENSITY);
    max7219.showDashes();
    Serial.println(F("[MAX7219] OK"));
  } else {
    Serial.println(F("[MAX7219] skip/fail"));
  }
  holdBootSafePins();

  Serial.println(F("[BOOT] OLED..."));
  data.oledOK = OledDisplay::begin();
  if (data.oledOK) {
    OledDisplay::showBoot("Indoor", FIRMWARE_VERSION);
  }

  Serial.println(F("[BOOT] LCD..."));
  data.lcdOK = LcdUi::begin();
  if (data.lcdOK) {
    LcdUi::showBoot();
  }

  displaysReady = true;
  Serial.println(F("[BOOT] displays diferidos OK"));
}

static void taskWiFiCallback() { stationWiFi.update(); }
static void taskNtpCallback() { NtpTime::update(); }
static void taskMqttCallback() { MqttSubscriber::update(); }

static void taskClockCallback() {
  if (!displaysReady || !IndoorData::instance().max7219OK) return;
  if (NtpTime::isSynced()) {
    max7219.showTime(NtpTime::hours(), NtpTime::minutes(), NtpTime::seconds());
  } else {
    max7219.showDashes();
  }
}

static void taskOledCallback() {
  if (!displaysReady) return;
  OledDisplay::showStatus(IndoorData::instance());
}

static void taskLcdCallback() {
  if (!displaysReady) return;
  LcdUi::render(IndoorData::instance());
}

static void taskSystemCallback() {
  IndoorData& data = IndoorData::instance();
  data.uptime = (millis() - bootMillis) / 1000UL;
  data.freeHeap = ESP.getFreeHeap();
}