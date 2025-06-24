#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_heap_caps.h>

#define WRONG_CMD_CODE    0xFF

#define CAPTURE_CMD       0x01
#define SEND_CAM_DATA_CMD 0x02

#define TURN_ON_CAM_CODE  0x01
#define TURN_OFF_CAM_CODE 0x02

extern volatile bool recv_cmd_flag;

extern int update_comm(void);
extern void handle_cmd(void);

void pack_ack(uint8_t ack_code);
void pack_error(uint8_t error_code);
void pack_data(uint8_t* data, uint64_t len);

void send_reply(uint8_t* data, uint64_t len);
void send_photo(uint8_t* data, uint64_t len);
