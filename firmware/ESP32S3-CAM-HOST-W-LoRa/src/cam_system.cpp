#include "cam_system.h"

void system_init(void) {
  camadapter_init(); // Initialize the camera adapter
  turnoffallcams();  // Ensure all cameras are off at startup

  Serial.begin(250000);
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial) {
    delay(100);
  }

  Serial.println("System initializing...");

  lora_setup(); // Initialize LoRa communication

  while (!Serial || !Serial1) {
    delay(100);
  }

  delay(1000); // Allow some time for the system to stabilize

  // Serial.println("System ready. Waiting for commands...");

  // flush Serial1 to clear any initial data
  // while (Serial1.available()) {
  //   Serial1.read(); // Clear any initial data in the buffer
  // }
}