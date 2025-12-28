#include "cam_system.h"

// Dedicated SPI bus for SD card (SPI1 on ESP32-S3)
static SPIClass sdSPI(FSPI);

void SD_init(void) {
  // Enable internal pull-ups on SPI pins
  pinMode(SD_SCK, INPUT_PULLUP);
  pinMode(SD_MISO, INPUT_PULLUP);
  pinMode(SD_MOSI, INPUT_PULLUP);
  pinMode(SD_CS, INPUT_PULLUP);

  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdSPI)) {
    while (1);
  }
}

void system_init(void) {
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial1) { delay(10); }

  delay(100); // Allow some time for the system to stabilize

  SD_init();        // Initialize the SD card

  camadapter_init(); // Initialize the camera adapter
  turnoffallcams();  // Ensure all cameras are off at startup

  delay(100); // Allow some time for the system to stabilize

  lora_setup(); // Initialize LoRa communication

  // flush Serial1 to clear any initial data
  while (Serial1.available()) {
    Serial1.read(); // Clear any initial data in the buffer
  }
}
