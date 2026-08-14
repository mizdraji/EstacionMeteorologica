#include "HistoryBuffer.h"
#include "WeatherData.h"
#include "../network/NtpTime.h"

HistorySample HistoryBuffer::_samples[HISTORY_CAPACITY];
uint16_t HistoryBuffer::_head = 0;
uint16_t HistoryBuffer::_count = 0;

void HistoryBuffer::begin() {
  _head = 0;
  _count = 0;
}

uint16_t HistoryBuffer::count() {
  return _count;
}

uint16_t HistoryBuffer::capacity() {
  return HISTORY_CAPACITY;
}

uint32_t HistoryBuffer::intervalSec() {
  return HISTORY_INTERVAL_MS / 1000UL;
}

bool HistoryBuffer::getSample(uint16_t indexFromOldest, HistorySample& out) {
  if (indexFromOldest >= _count) {
    return false;
  }

  uint16_t start = (_head + HISTORY_CAPACITY - _count) % HISTORY_CAPACITY;
  uint16_t idx = (start + indexFromOldest) % HISTORY_CAPACITY;
  out = _samples[idx];
  return true;
}

void HistoryBuffer::pushFromWeatherData() {
  const WeatherData& data = WeatherData::instance();

  if (!weatherValueIsValid(data.temperatureMain) &&
      !weatherValueIsValid(data.humidity) &&
      !weatherValueIsValid(data.pressure)) {
    return;
  }

  HistorySample sample;
  sample.epoch = NtpTime::isSynced() ? NtpTime::epoch() : 0;
  sample.temperature = weatherValueIsValid(data.temperatureMain)
                           ? data.temperatureMain
                           : WEATHER_VALUE_INVALID;
  sample.humidity = weatherValueIsValid(data.humidity) ? data.humidity : WEATHER_VALUE_INVALID;
  sample.pressure = weatherValueIsValid(data.pressure) ? data.pressure : WEATHER_VALUE_INVALID;

  _samples[_head] = sample;
  _head = (_head + 1) % HISTORY_CAPACITY;
  if (_count < HISTORY_CAPACITY) {
    _count++;
  }
}
