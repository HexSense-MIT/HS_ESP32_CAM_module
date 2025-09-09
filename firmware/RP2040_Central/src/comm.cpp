#include "comm.h"

volatile bool recv_cmd_flag = false;
uint8_t recv_cmd[20] = {0};
uint8_t recv_ack[150] = {0};

void comm_init(void) {
  Serial.begin(250000);
  while (!Serial) {;}
}

int update_comm(CommandSource source) {
  int i = 0;

  if (source == FROM_GUI) {
    while (Serial.available()) {
      recv_cmd[i++] = Serial.read();
    }
  } else {
    while (LoRa.available()) {
      recv_ack[i++] = LoRa.read();
    }
  }

  return i; // Return the number of bytes read
}
