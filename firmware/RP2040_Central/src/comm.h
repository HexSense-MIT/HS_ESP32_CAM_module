#pragma once

#include <Arduino.h>

#include "central_lora.h"

extern volatile bool recv_cmd_flag;

enum CommandSource{
  FROM_GUI = 0,
  FRAM_HEXSENSE
};

extern uint8_t recv_cmd[20];
extern uint8_t recv_ack[150];

extern void comm_init(void);

extern int update_comm(CommandSource source);

