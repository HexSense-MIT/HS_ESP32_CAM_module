#include "cam_system.h"

void SD_init(void) {
  // Enable internal pull-ups on SPI pins
  pinMode(SD_SCK, INPUT_PULLUP);
  pinMode(SD_MISO, INPUT_PULLUP);
  pinMode(SD_MOSI, INPUT_PULLUP);
  pinMode(SD_CS, INPUT_PULLUP);

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS)) {
    // Serial.println("SD init failed");
    while (1);
  }

  // uint8_t cardType = SD.cardType();

  // Serial.print("Card Type: ");
  // if (cardType == CARD_MMC) Serial.println("MMC");
  // else if (cardType == CARD_SD) Serial.println("SDSC");
  // else if (cardType == CARD_SDHC) Serial.println("SDHC");
  // else Serial.println("UNKNOWN");

  // Total size in MB
  // uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  // Serial.printf("Card Size: %llu MB\n", cardSize);

  // Serial.println("SD ready!");
}

void system_init(void) {
  Serial.begin(250000);
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial || !Serial1) {
    delay(100);
  }

  delay(100); // Allow some time for the system to stabilize

  SD_init();        // Initialize the SD card

  camadapter_init(); // Initialize the camera adapter
  turnoffallcams();  // Ensure all cameras are off at startup

  delay(100); // Allow some time for the system to stabilize

  // Serial.println("System ready. Waiting for commands...");

  // flush Serial1 to clear any initial data
  while (Serial1.available()) {
    Serial1.read(); // Clear any initial data in the buffer
  }
}