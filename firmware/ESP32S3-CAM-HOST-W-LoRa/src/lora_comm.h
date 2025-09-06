#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include "cam_system.h"
#include "cam_comm.h"

#define LORA_CS_PIN 44
#define LORA_RST     7
#define LORA_IRQ    10

extern void lora_setup(void);
extern void lora_send_data(uint8_t* data, size_t len);
extern size_t lora_receive_data(uint8_t* buf, size_t buf_size);

