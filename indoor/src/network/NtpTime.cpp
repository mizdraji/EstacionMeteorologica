#include "NtpTime.h"
#include "../Config.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET_SEC,
                            NTP_UPDATE_INTERVAL_MS);
static bool ntpStarted = false;

void NtpTime::begin() {
  if (ntpStarted) {
    return;
  }
  timeClient.begin();
  ntpStarted = true;
}

void NtpTime::update() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (!ntpStarted) {
    begin();
  }
  timeClient.update();
}

bool NtpTime::isSynced() {
  return ntpStarted && timeClient.getEpochTime() > 100000UL;
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
