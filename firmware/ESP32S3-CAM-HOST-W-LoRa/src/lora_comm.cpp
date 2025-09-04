#include "lora_comm.h"

RH_RF95 rf95(LORA_CS_PIN, LORA_IRQ);

static void lora_pin_init(void) {
  pinMode(LORA_PWR_EN, OUTPUT);
  pinMode(LORA_RST,    OUTPUT);
  pinMode(LORA_CS_PIN, OUTPUT);
  pinMode(LORA_IRQ,    INPUT);
}

void lora_poweron(void) {
  digitalWrite(LORA_PWR_EN, HIGH);
  delay(10);
}

void lora_rst(void) {
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(10);
}

void lora_init(void) {
  lora_pin_init();

  lora_poweron();
  lora_rst();

  rf95.init();
}

void lora_send_data(uint8_t* data, size_t len) {
  rf95.send(data, len);
  rf95.waitPacketSent();
}

size_t lora_receive_data(uint8_t* buf, size_t buf_size) {
  size_t received_len = 0;

  if (rf95.available()) {
    if (rf95.recv(buf, (uint8_t*)&buf_size)) {
      received_len = buf_size;
    } else {
      received_len = 0;
    }
  }
  return received_len;
}







