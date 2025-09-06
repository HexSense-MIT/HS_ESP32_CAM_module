#include "lora_comm.h"

static void lora_pin_init(void) {
  pinMode(LORA_RST,    OUTPUT);
  pinMode(LORA_CS_PIN, OUTPUT);
  pinMode(LORA_IRQ,    INPUT);
}

void lora_rst(void) {
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(10);
}

void lora_setup (void) {
  lora_pin_init();
  lora_rst();
  LoRa.setPins(LORA_CS_PIN, LORA_RST, LORA_IRQ);
  SPI.begin();

  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
}

void lora_send_data(uint8_t* data, size_t len) {
  LoRa.beginPacket();
  LoRa.write(data, len);
  LoRa.endPacket();

  Serial.println("LoRa packet sent");
}

size_t lora_receive_data(uint8_t* buf, size_t buf_size) {
  size_t received_len = 0;

  return received_len;
}







