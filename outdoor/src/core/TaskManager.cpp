#include "TaskManager.h"
#include "../Config.h"
#include "SensorManager.h"
#include "WeatherData.h"
#include "../network/StationWiFi.h"
#include "../network/WebServerManager.h"
#include "../network/OTAManager.h"
#include "../network/SerialDiagnostics.h"
#include "../network/NtpTime.h"
#include "../network/ExternalWeather.h"
#include "../network/MqttPublisher.h"
#include "HistoryBuffer.h"
#include <TaskScheduler.h>

static Scheduler scheduler;
static SensorManager sensorManager;
static StationWiFi stationWiFi;

static unsigned long bootMillis = 0;

static void taskAHTCallback();
static void taskBMPCallback();
static void taskWiFiCallback();
static void taskWebCallback();
static void taskOTACallback();
static void taskSerialCallback();
static void taskSystemCallback();
static void taskNtpCallback();
static void taskHistoryCallback();
static void taskExternalWeatherCallback();
static void taskMqttCallback();

static Task taskAHT(AHT_INTERVAL_MS, TASK_FOREVER, &taskAHTCallback);
static Task taskBMP(BMP180_INTERVAL_MS, TASK_FOREVER, &taskBMPCallback);
static Task taskWiFi(WIFI_INTERVAL_MS, TASK_FOREVER, &taskWiFiCallback);
static Task taskWeb(WEB_INTERVAL_MS, TASK_FOREVER, &taskWebCallback);
static Task taskOTA(OTA_INTERVAL_MS, TASK_FOREVER, &taskOTACallback);
static Task taskSerial(SERIAL_INTERVAL_MS, TASK_FOREVER, &taskSerialCallback);
static Task taskSystem(SYSTEM_INTERVAL_MS, TASK_FOREVER, &taskSystemCallback);
static Task taskNtp(NTP_TASK_INTERVAL_MS, TASK_FOREVER, &taskNtpCallback);
static Task taskHistory(HISTORY_INTERVAL_MS, TASK_FOREVER, &taskHistoryCallback);
static Task taskExternalWeather(OWM_TASK_INTERVAL_MS, TASK_FOREVER, &taskExternalWeatherCallback);
static Task taskMqtt(MQTT_TASK_INTERVAL_MS, TASK_FOREVER, &taskMqttCallback);

void TaskManager::begin() {
  bootMillis = millis();

  WeatherData::instance().init();
  HistoryBuffer::begin();
  sensorManager.begin();

  stationWiFi.begin();
  ExternalWeather::begin();
  MqttPublisher::begin();

  scheduler.addTask(taskAHT);
  scheduler.addTask(taskBMP);
  scheduler.addTask(taskWiFi);
  scheduler.addTask(taskWeb);
  scheduler.addTask(taskOTA);
  scheduler.addTask(taskSerial);
  scheduler.addTask(taskSystem);
  scheduler.addTask(taskNtp);
  scheduler.addTask(taskHistory);
  scheduler.addTask(taskExternalWeather);
  scheduler.addTask(taskMqtt);

  taskAHT.enable();
  taskBMP.enable();
  taskWiFi.enable();
  taskWeb.enable();
  taskOTA.enable();
  taskSerial.enable();
  taskSystem.enable();
  taskNtp.enable();
  taskHistory.enable();
  taskExternalWeather.enable();
  taskMqtt.enable();

  SerialDiagnostics::printBootBanner();
}

void TaskManager::run() {
  scheduler.execute();
}

static void taskAHTCallback() {
  sensorManager.readAHT10();
}

static void taskBMPCallback() {
  sensorManager.readBMP180();
}

static void taskWiFiCallback() {
  stationWiFi.update();
}

static void taskWebCallback() {
  WebServerManager::handle();
}

static void taskOTACallback() {
  OTAManager::handle();
}

static void taskSerialCallback() {
  SerialDiagnostics::printPeriodicStatus();
}

static void taskSystemCallback() {
  WeatherData& data = WeatherData::instance();
  data.uptime = (millis() - bootMillis) / 1000UL;
  data.freeHeap = ESP.getFreeHeap();
}

static void taskNtpCallback() {
  NtpTime::update();
  WeatherData::instance().ntpSynced = NtpTime::isSynced();
}

static void taskHistoryCallback() {
  HistoryBuffer::pushFromWeatherData();
}

static void taskExternalWeatherCallback() {
  ExternalWeather::update();
}

static void taskMqttCallback() {
  MqttPublisher::update();
}
