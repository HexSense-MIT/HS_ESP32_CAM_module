#include "cam_system.h"
#include "cam_comm.h"
#include "cam_adapter.h"

volatile bool recv_cmd_flag = false;

uint8_t recv_cmd[5] = {0, 0, 0, 0, 0};
uint8_t reply_data[9] = {0xAA, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t cam_num = 0;
uint8_t cam_data = 0;

uint64_t data_len = 0; // Length of the data to be sent

int recv_data_i = 0;

void pack_ack(uint8_t ack_code) {
  reply_data[1] = cam_num;
  reply_data[2] = ack_code; // Set the acknowledgment code
}

void pack_error(uint8_t error_code) {
  reply_data[1] = error_code;
}

void pack_data(uint8_t* data, uint64_t len) {
  memcpy(reply_data + 1, data, len);
}

void send_reply(uint8_t* data, uint64_t len) {
  Serial.write(reply_data, len); // Send the reply data including the header
  Serial.flush(); // Ensure the data is sent immediately
}

int update_comm(void) {
  int i = 0;

  while (Serial.available()) {
    recv_cmd[i++] = Serial.read();
  }

  if (recv_cmd[0] == 0xAA && i == 5) {
    recv_cmd_flag = true;
  }
  else {
    recv_cmd_flag = false; // Reset the flag if the command is not valid
  }

  return i; // Return the number of bytes read

}

void handle_cmd(void) {
  if (recv_cmd_flag) { // command is correct length
    recv_cmd_flag = false; // Reset the flag

    if (recv_cmd[2] == TURN_ON_CAM_CODE) { // turn on the camera
      turnoffallcams(); // Ensure all cameras are off before turning on a specific one
      delay(10);

      cam_num = recv_cmd[1]; // Get the camera number from the command

      if (cam_num < 6) {
        turnoncam(cam_num + 1); // Call the function to turn on the camera
        pack_ack(TURN_ON_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_data, sizeof(reply_data)); // Send acknowledgment reply
        delay(1000);    // Wait for 1 second to ensure the camera is powered on
        flush_buffer(); // Flush the serial buffer to clear any remaining data
      }
      else if (cam_num == 6) { // turn on all cameras
        for (int i = 0; i < 6; i++) {
          turnoncam(i + 1); // Call the function to turn on each camera
        }
        pack_ack(TURN_ON_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_data, sizeof(reply_data)); // Send acknowledgment reply
        delay(1000);    // Wait for 1 second to ensure all cameras are powered on
        flush_buffer(); // Flush the serial buffer to clear any remaining data
      }
      else { // not allowed to turn on all cameras at once
        pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
        send_reply(reply_data, sizeof(reply_data)); // Send error reply
      }
    }
    else if (recv_cmd[2] == TURN_OFF_CAM_CODE) { // turn off the camera
      cam_num = recv_cmd[1]; // Get the camera number from the command

      if (cam_num < 6) {
        turnoffcam(cam_num + 1); // Call the function to turn off the camera
        pack_ack(TURN_OFF_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_data, sizeof(reply_data)); // Send acknowledgment reply
      }
      else if (cam_num == 6) { // turn off all cameras
        turnoffallcams(); // Call the function to turn off all cameras
        pack_ack(TURN_OFF_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_data, sizeof(reply_data)); // Send acknowledgment reply
      }
      else {
        // Serial.println("Invalid camera number. Must be between 0 and 5.");
        pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
        send_reply(reply_data, sizeof(reply_data)); // Send error reply
        return;
      }
    }
    else if (recv_cmd[2] == 0x03) { // take a photo
      Serial1.write(CAPTURE_CMD);
      Serial1.flush();

      while (!Serial1.available()) {;}

      recv_data_i = 0;
      while (Serial1.available()) {
        cam_data = Serial1.read();
        reply_data[recv_data_i++] = cam_data; // Store the length of the data
      }
      data_len = reply_data[1] | (reply_data[2] << 8) | (reply_data[3] << 16) | (reply_data[4] << 24);
      send_reply(reply_data, sizeof(reply_data)); // Send acknowledgment reply
    }
    else if (recv_cmd[2] == 0x04) { // send photo data
      Serial1.write(SEND_CAM_DATA_CMD);
      Serial1.flush();

      recv_data_i = 0;

      while (recv_data_i < data_len) {
        if (Serial1.available()) {
          Serial.write(Serial1.read());
          recv_data_i++;
        }
      }
    }
    else {
      // Serial.println("Invalid command received.");
      pack_error(WRONG_CMD_CODE); // Pack error for invalid command
      send_reply(reply_data, sizeof(reply_data)); // Send error reply
    }
  }
  else { // command is not correct length
    // Serial.println("Invalid command length received.");
    pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
    send_reply(reply_data, sizeof(reply_data)); // Send error reply
  }
}