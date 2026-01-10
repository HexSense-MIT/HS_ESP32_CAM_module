// comm with GUI through USB.
// Store image data in PSRAM first then into SD card.
#include <Arduino.h>

#include "cam_system.h"
#include "cam_comm.h"
#include <FS.h>
#include <SD_MMC.h>

int     current_cam_num = 1; // Default camera number
int32_t file_name_num   = 1;

void setup() {
  system_init();
}

void loop() {
  delay(10000); // Wait for 2 seconds before the next iteration

  if (current_cam_num == 7) {
    current_cam_num = 1;
  }

  turn_on_a_camera(current_cam_num);
  take_photos(3);

  char filename[20];
  sprintf(filename, "/%d.jpg", file_name_num);
  // Ensure unique filename
  while (SD.exists(filename)) {
    file_name_num ++;
    sprintf(filename, "/%d.jpg", file_name_num);
  }
  store_img2sd(filename);

  turn_off_a_camera(current_cam_num);

  current_cam_num ++;

  // if (update_comm()) {
  //   handle_cmd();
  // }
}





