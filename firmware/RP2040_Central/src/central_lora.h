#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define LORA_CS_PIN     1
#define LORA_SCK_PIN    2
#define LORA_MOSI_PIN   3
#define LORA_MISO_PIN   4

#define LORA_RESET_PIN 27
#define LORA_DIO0_PIN  26

extern void lora_init(void);

extern void lora_send(uint8_t *data, uint8_t len);
extern void serial_send(uint8_t *data, uint8_t len);