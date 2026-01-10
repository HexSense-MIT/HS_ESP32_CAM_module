#include <Arduino.h>

#define CAM_RX_PIN 7
#define CAM_TX_PIN 6

void setup() {
  Serial.begin(250000);
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial || !Serial1) {
    delay(100);
  }
}

void loop() {
  while (Serial.available()) {
    uint8_t recv = Serial.read();
    Serial1.write(recv);
  }
  while (Serial1.available()) {
    uint8_t send = Serial1.read();
    Serial.write(send);
  }
}
