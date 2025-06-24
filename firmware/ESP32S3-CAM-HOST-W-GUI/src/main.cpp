#include <Arduino.h>

#include "cam_system.h"
#include "cam_comm.h"

void setup() {
  system_init();
}

void loop() {
  if (update_comm()) {
    handle_cmd();
  }
}
