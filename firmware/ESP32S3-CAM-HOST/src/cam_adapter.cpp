#include "cam_adapter.h"

SFE_PCA95XX io; // Global instance of the PCA95XX I2C expander

void camadapter_init(void) {
  Wire.begin(39, 40);

  // Initialize the PCA95XX I2C expander
  if (io.begin(PCA9554_ADDRESS_21) == false) {
    Serial.println("PCA95xx not detected. Please check wiring. Freezing...");
    while (true);
  }

  // Set all camera pins to OUTPUT mode
  for (int i = 0; i < 8; i++) {
    io.pinMode(i, OUTPUT);
  }

  Serial.println("Camera adapter initialized.");
}

void turnoncam(int cam) {
  if (cam < 0 || cam >= 8) {
    Serial.println("Invalid camera index. Must be between 0 and 7.");
    return;
  }

  // Set the corresponding pin to HIGH to turn on the camera
  io.digitalWrite(cam, HIGH);
  Serial.print("Camera ");
  Serial.print(cam);
  Serial.println(" turned ON.");
}

void turnoffallcams(void) {
  // Turn off all cameras by setting all pins to LOW
  for (int i = 0; i < 8; i++) {
    io.digitalWrite(i, LOW);
  }
  Serial.println("All cameras turned OFF.");
}

void turnoffcam(int cam) {
  if (cam < 0 || cam >= 8) {
    Serial.println("Invalid camera index. Must be between 0 and 7.");
    return;
  }

  // Set the corresponding pin to LOW to turn off the camera
  io.digitalWrite(cam, LOW);
  Serial.print("Camera ");
  Serial.print(cam);
  Serial.println(" turned OFF.");
}
