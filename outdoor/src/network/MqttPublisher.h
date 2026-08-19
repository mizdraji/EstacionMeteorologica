#pragma once

// Publicación periódica MQTT (PubSubClient). Ver docs/mqtt.md.
class MqttPublisher {
public:
  static void begin();
  static void update();
};
