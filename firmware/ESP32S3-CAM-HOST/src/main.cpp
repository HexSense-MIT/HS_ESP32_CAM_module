#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_heap_caps.h>

#define CAM_RX_PIN 13
#define CAM_TX_PIN 12

#define CAPTURE_CMD       0x01
#define SEND_CAM_DATA_CMD 0x02

uint8_t camdatalen[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cam_data = 0;

// allocate a 1MB buffer for the camera data
#define CAM_BUFFER_SIZE 512 * 1024
uint8_t* camBuffer = (uint8_t*)heap_caps_malloc(CAM_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

int i = 0;
int j = 0;

void setup() {
  Serial.begin(250000);
  Serial1.begin(250000, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  while (!Serial || !Serial1) {
    delay(100);
  }

  Serial.println("Camera Serial Bridge started. Waiting for commands...");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
    case '1':
      Serial1.write(CAPTURE_CMD);
      Serial1.flush(); // Ensure the command is sent before proceeding

      while (!Serial1.available()) {
        // Wait for the camera to respond
        delay(10);
      }

      while (Serial1.available()) {
        cam_data = Serial1.read();
        Serial.write(cam_data);
      }

      break;

    case '2':
      Serial1.write(SEND_CAM_DATA_CMD);
      Serial1.flush(); // Ensure the command is sent before proceeding

      while (!Serial1.available()) {
        // Wait for the camera to respond
        delay(10);
      }

      i = 0;

      while (Serial1.available()) {
        // Read and forward camera data to Serial0
        cam_data = Serial1.read();
        Serial.write(cam_data);
        // camBuffer[i++] = data;
      }

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
