// comm with GUI through USB.
// Store image data in PSRAM first then into SD card.
#include <Arduino.h>

#include "cam_system.h"
#include "cam_comm.h"
#include <FS.h>
#include <SD_MMC.h>

void setup() {
  system_init();
}

void loop() {
  if (update_comm()) {
    handle_cmd();
  }
}





