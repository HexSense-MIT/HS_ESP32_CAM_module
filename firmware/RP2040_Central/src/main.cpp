#include <Arduino.h>

#include "central_lora.h"

void setup() {
  Serial.begin(250000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Serial OK!");
  lora_init();
}

void loop() {
  static uint32_t counter = 0;
  counter++;
  Serial.printf("Counter: %lu\n", counter);
  lora_send_test((uint8_t *)&counter, sizeof(counter));
  delay(1000);
}
