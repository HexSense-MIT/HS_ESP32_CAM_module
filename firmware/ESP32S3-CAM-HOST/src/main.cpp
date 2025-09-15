#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_heap_caps.h>

#include "cam_adapter.h"

#define CAM_RX_PIN 13
#define CAM_TX_PIN 12

#define CAPTURE_CMD       0x01
#define SEND_CAM_DATA_CMD 0x02

uint8_t camdatalen[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cam_data = 0;

uint64_t data_len = 0;

uint8_t recv_cmd[5] = {0, 0, 0, 0, 0};

// allocate a 1MB buffer for the camera data
#define CAM_BUFFER_SIZE 512 * 1024
uint8_t* camBuffer = (uint8_t*)heap_caps_malloc(CAM_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

int i = 0;
int j = 0;
bool recv_cmd_flag = false;

void setup() {
  camadapter_init(); // Initialize the camera adapter
  turnoffallcams();  // Ensure all cameras are off at startup

  Serial.begin(250000);
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial || !Serial1) {
    delay(100);
  }

  turnoncam(2);
  delay(1000); // Wait for the camera to initialize

  Serial.println("Camera Serial Bridge started. Waiting for commands...");

  // flsuh Serial1 to clear any initial data
  while (Serial1.available()) {
    Serial1.read(); // Clear any initial data in the buffer
  }
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
    case '1':
      Serial1.write(CAPTURE_CMD);
      // Serial1.flush(); // Ensure the command is sent before proceeding

      while (!Serial1.available()) {
        // Wait for the camera to respond
        delay(10);
      }

      i = 0;

      while (Serial1.available()) {
        cam_data = Serial1.read();
        camdatalen[i++] = cam_data; // Store the length of the data
      }

      if (camdatalen[0] == 0xAA) {
        data_len = camdatalen[1] | (camdatalen[2] << 8) | (camdatalen[3] << 16) | (camdatalen[4] << 24);
        Serial.print("Data length: ");
        Serial.println(data_len);
      }
      else {
        Serial.println("Invalid data length received from camera.");
      }

      break;

    case '2':
      Serial1.write(SEND_CAM_DATA_CMD);
      // Serial1.flush(); // Ensure the command is sent before proceeding

      while (!Serial1.available()) {
        // Wait for the camera to respond
        delay(10);
      }

      i = 0;

      while (i < data_len) {
        if (Serial1.available()) {
          cam_data = Serial1.read();
          printf("%02X", cam_data);
          // Serial.write(cam_data);
          i++;
          // camBuffer[i++] = cam_data;
        }
        yield();
      }

      Serial.println();
      Serial.print("i = "); // Add a newline after sending the data
      Serial.println(i);

      // print the whole buffer to Serial0 with elegant way
      // for (j = 0; j < i; j++) {
      //   Serial.write(camBuffer[j]);
      // }

      // Serial.println(); // Add a newline after sending the data

      break;

    default:
      break;
    }
  }
}
