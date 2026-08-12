#include "NtpTime.h"
#include "../Config.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);
static bool _ntpStarted = false;

void NtpTime::begin() {
  if (_ntpStarted) {
    return;
  }
  timeClient.begin();
  _ntpStarted = true;
}

void NtpTime::update() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!_ntpStarted) {
    begin();
  }

  timeClient.update();
}

bool NtpTime::isSynced() {
  return _ntpStarted && timeClient.getEpochTime() > 100000UL;
}

uint8_t NtpTime::hours() {
  return (uint8_t)timeClient.getHours();
}

uint8_t NtpTime::minutes() {
  return (uint8_t)timeClient.getMinutes();
}

uint8_t NtpTime::seconds() {
  return (uint8_t)timeClient.getSeconds();
}

uint32_t NtpTime::epoch() {
  return (uint32_t)timeClient.getEpochTime();
}

void NtpTime::formatTime(char* buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize < 9) {
    return;
  }
  if (!isSynced()) {
    snprintf(buffer, bufferSize, "--:--:--");
    return;
  }
  snprintf(buffer, bufferSize, "%02u:%02u:%02u", hours(), minutes(), seconds());
}
