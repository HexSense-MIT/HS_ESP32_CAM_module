#include <Arduino.h>

#include "cam_system.h"
#include "cam_comm.h"

void setup() {
  system_init();
}

void loop() {
  lora_send_data((uint8_t *)"Hello", 5);
  delay(1000);
  // if (update_comm()) {
  //   handle_cmd();
  // }
}





