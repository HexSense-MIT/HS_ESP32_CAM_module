#include "central_lora.h"

void lora_init(void) {
  SPI.setMISO(LORA_MISO_PIN);
  SPI.setMOSI(LORA_MOSI_PIN);
  SPI.setSCK(LORA_SCK_PIN);
  SPI.setCS(LORA_CS_PIN);

  SPI.begin();
  LoRa.setPins(LORA_CS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);

  while (!LoRa.begin(915E6)) {
    delay(10);
  }
}

void lora_send(uint8_t *data, uint8_t len) {
  LoRa.beginPacket();
  LoRa.write(data, len);
  LoRa.endPacket();
}

void serial_send(uint8_t *data, uint8_t len) {
  Serial.write(data, len);
}