#include <Arduino.h>

#include "cam_system.h"
#include "cam_comm.h"

uint8_t recv_byte = 0;

void setup() {
  system_init();
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print(LoRa.read(), HEX);
      Serial.print(" ");
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}





