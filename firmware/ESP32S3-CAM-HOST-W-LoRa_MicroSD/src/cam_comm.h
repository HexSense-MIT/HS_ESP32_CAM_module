#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_heap_caps.h>

#include "lora_comm.h"

#define WRONG_CMD_CODE    0xFF

#define CAPTURE_CMD       0x01
#define SEND_CAM_DATA_CMD 0x02

#define TURN_ON_CAM_CODE    0x01
#define TURN_OFF_CAM_CODE   0x02
#define TAKE_PHOTO_CAM_CODE 0x03
#define GRAB_DATA_CAM_CODE  0x04

#define SD_DONE_CODE        0x0D

#define IMG_SIZE 1024 * 500  // 100 KB buffer for image data

extern volatile bool recv_cmd_flag;

extern int update_comm(void);
extern void handle_cmd(void);

size_t pack_ack(uint8_t ack_code);
size_t pack_error(uint8_t error_code);
size_t pack_data_cobs(uint8_t* data, uint64_t len);
size_t pack_data_raw(uint8_t* data, uint64_t len);

void send_reply(uint8_t* data, size_t len);

size_t cobs_encode(const uint8_t* input, size_t length, uint8_t* output);
size_t cobs_decode(const uint8_t* input, size_t length, uint8_t* output);

