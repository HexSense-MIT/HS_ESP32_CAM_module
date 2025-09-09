#include <Arduino.h>

#include "comm.h"
#include "central_lora.h"

void setup() {
  comm_init();
  lora_init();
}

void loop() {
  if (int len = update_comm(FROM_GUI); len > 0) {
    lora_send(recv_cmd, len);
  }
  if (int len = update_comm(FRAM_HEXSENSE); len > 0) {
    serial_send(recv_ack, len);
  }
}


