#include <Arduino.h>
#include "Config.h"
#include "core/TaskManager.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  TaskManager::begin();
}

void loop() {
  TaskManager::run();
}
