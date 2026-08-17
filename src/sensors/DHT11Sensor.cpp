#include "DHT11Sensor.h"
#include "../Config.h"

bool DHT11Sensor::begin() {
  pinMode(DHT11_PIN, INPUT_PULLUP);
  _initialized = true;
  return true;
}

bool DHT11Sensor::readSample(int& humidity, int& temperature) {
  uint8_t bits[5] = {0};
  uint8_t bitIndex = 0;
  uint8_t byteIndex = 0;

  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(18);
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT11_PIN, INPUT_PULLUP);

  unsigned int timeout = micros() + 100;
  while (digitalRead(DHT11_PIN) == HIGH) {
    if (micros() > timeout) {
      return false;
    }
  }

  timeout = micros() + 100;
  while (digitalRead(DHT11_PIN) == LOW) {
    if (micros() > timeout) {
      return false;
    }
  }

  timeout = micros() + 100;
  while (digitalRead(DHT11_PIN) == HIGH) {
    if (micros() > timeout) {
      return false;
    }
  }

  for (uint8_t i = 0; i < 40; i++) {
    timeout = micros() + 100;
    while (digitalRead(DHT11_PIN) == LOW) {
      if (micros() > timeout) {
        return false;
      }
    }

    unsigned long highStart = micros();
    timeout = highStart + 100;
    while (digitalRead(DHT11_PIN) == HIGH) {
      if (micros() > timeout) {
        return false;
      }
    }

    unsigned long duration = micros() - highStart;
    bits[byteIndex] <<= 1;
    if (duration > 50) {
      bits[byteIndex] |= 1;
    }

    bitIndex++;
    if (bitIndex == 8) {
      bitIndex = 0;
      byteIndex++;
    }
  }

  uint8_t checksum = bits[0] + bits[1] + bits[2] + bits[3];
  if (checksum != bits[4]) {
    return false;
  }

  humidity = bits[0];
  temperature = bits[2];
  return true;
}

void DHT11Sensor::read(WeatherData& data) {
  if (!_initialized) {
    data.dht11OK = false;
    return;
  }

  int humidity = 0;
  int temperature = 0;

  if (!readSample(humidity, temperature)) {
    data.dht11OK = false;
    data.updateDerivedValues();
    return;
  }

  if (humidity < 0 || humidity > 100 || temperature < -20 || temperature > 60) {
    data.dht11OK = false;
    data.updateDerivedValues();
    return;
  }

  data.temperatureDHT = (float)temperature;
  data.humidity = (float)humidity;
  data.dht11OK = true;
  data.updateDerivedValues();
}
