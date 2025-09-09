#include "cam_system.h"
#include "cam_comm.h"
#include "cam_adapter.h"

volatile bool recv_cmd_flag     = false;

uint8_t recv_cmd[10]            = {0};
uint8_t recv_cmd_decoded[10]    = {0};
uint8_t reply_ack[15]           = {0xAA};
uint8_t reply_ack_encoded[15]   = {0x00};

uint8_t reply_data[120]         = {0xBB};
uint8_t reply_data_encoded[120] = {0xBB};

uint8_t cam_module_ack[10]      = {0};
uint8_t cam_module_data[150]    = {0};

uint8_t *img_buffer = (uint8_t*) heap_caps_malloc(IMG_SIZE*(sizeof(uint8_t)), MALLOC_CAP_SPIRAM);

uint8_t cam_num = 0;
uint8_t cam_data = 0;

uint64_t data_len = 0; // Length of the data to be sent

int recv_data_i = 0;

size_t pack_ack(uint8_t ack_code) {
  reply_ack[1] = cam_num;
  reply_ack[2] = ack_code; // Set the acknowledgment code
  size_t encoded_len = cobs_encode(reply_ack, 7, reply_ack_encoded);
  reply_ack_encoded[encoded_len++] = 0x00; // Append the delimiter

  return encoded_len;
}

size_t pack_error(uint8_t error_code) {
  reply_ack[1] = error_code;
  size_t encoded_len = cobs_encode(reply_ack, 7, reply_ack_encoded);
  reply_ack_encoded[encoded_len++] = 0x00; // Append the delimiter

  return encoded_len;
}

size_t pack_data(uint8_t* data, uint64_t len, uint8_t cam_num, uint8_t seq_num) {
  reply_data[1] = cam_num;
  reply_data[2] = seq_num;
  memcpy(reply_data + 3, data, len);

  size_t encoded_len = cobs_encode(reply_ack, len + 3, reply_ack_encoded);
  reply_ack_encoded[encoded_len++] = 0x00;

  return encoded_len;
}

void send_reply(uint8_t* data, size_t len) {
  LoRa.beginPacket();
  LoRa.write(data, len);
  LoRa.endPacket();
}

int update_comm(void) {
  recv_data_i = 0;

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
     // read packet
    while (LoRa.available()) {
      recv_cmd[recv_data_i++] = LoRa.read();
    }

    size_t decoded_len = cobs_decode(recv_cmd, recv_data_i-1, recv_cmd_decoded);

    if (recv_cmd_decoded[0] == 0xAA && decoded_len == 5) {
      recv_cmd_flag = true;
      return 1; // Return the number of bytes read
    }
    else {
      recv_cmd_flag = false; // Reset the flag if the command is not valid
      return 0;
    }
  }
  return 0;
}

void handle_cmd(void) {
  if (recv_cmd_flag) {     // command is correct length
    recv_cmd_flag = false; // Reset the flag

    if (recv_cmd_decoded[2] == TURN_ON_CAM_CODE) { // turn on the camera
      turnoffallcams(); // Ensure all cameras are off before turning on a specific one
      delay(5);

      cam_num = recv_cmd_decoded[1]; // Get the camera number from the command

      if (cam_num < 6) {
        turnoncam(cam_num + 1); // Call the function to turn on the camera
        delay(500);             // Wait for 0.5 second to ensure the camera is powered on
        size_t ack_len = pack_ack(TURN_ON_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
        flush_buffer();                              // Flush the serial buffer to clear any remaining data
      }
      else if (cam_num == 6) { // turn on all cameras
        for (int i = 0; i < 6; i++) {
          turnoncam(i + 1); // Call the function to turn on each camera
        }
        delay(500);    // Wait for 0.5 second to ensure all cameras are powered on
        size_t ack_len = pack_ack(TURN_ON_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
        flush_buffer(); // Flush the serial buffer to clear any remaining data
      }
      else { // not allowed to turn on all cameras at once
        size_t ack_len = pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
      }
    }
    else if (recv_cmd_decoded[2] == TURN_OFF_CAM_CODE) { // turn off the camera
      cam_num = recv_cmd_decoded[1]; // Get the camera number from the command

      if (cam_num < 6) {
        turnoffcam(cam_num + 1); // Call the function to turn off the camera
        size_t ack_len = pack_ack(TURN_OFF_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
      }
      else if (cam_num == 6) { // turn off all cameras
        turnoffallcams(); // Call the function to turn off all cameras
        size_t ack_len = pack_ack(TURN_OFF_CAM_CODE); // Pack acknowledgment for successful operation
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
      }
      else {
        // Serial.println("Invalid camera number. Must be between 0 and 5.");
        size_t ack_len = pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
        send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
        return;
      }
    }
    else if (recv_cmd_decoded[2] == 0x03) { // take a photo
      Serial1.write(CAPTURE_CMD);
      Serial1.flush();

      while (!Serial1.available()) {;}

      recv_data_i = 0;
      while (Serial1.available()) {
        cam_data = Serial1.read();
        cam_module_ack[recv_data_i++] = cam_data; // Store the length of the data
      }
      data_len = cam_module_ack[1] | (cam_module_ack[2] << 8) | (cam_module_ack[3] << 16) | (cam_module_ack[4] << 24);
      reply_ack[1] = cam_num;
      reply_ack[2] = 0x03;
      memcpy(reply_ack + 3, cam_module_ack + 1, 4); // Copy the length of the data to the reply
      size_t encoded_len = cobs_encode(reply_ack, 7, reply_ack_encoded);
      reply_ack_encoded[encoded_len++] = 0x00; // Append the delimiter

      send_reply(reply_ack_encoded, encoded_len); // Send acknowledgment reply
    }
    else if (recv_cmd_decoded[2] == 0x04) { // send photo data
      Serial1.write(SEND_CAM_DATA_CMD);
      Serial1.flush();

      recv_data_i = 0;

      while (recv_data_i < data_len) {
        if (Serial1.available()) {
          img_buffer[recv_data_i++] = Serial1.read();
        }
      }

      uint8_t seq_num           = 0;
      size_t bytes_left         = data_len;
      size_t chunk_size         = 100;
      size_t current_chunk_size = 0;
      size_t encoded_len        = 0;
      size_t offset             = 0;

      while (bytes_left > 0) {
        current_chunk_size = (bytes_left < chunk_size) ? bytes_left : chunk_size;
        encoded_len = pack_data(img_buffer + offset, current_chunk_size, cam_num, seq_num);
        send_reply(reply_data_encoded, encoded_len); // Send data reply
        offset += current_chunk_size;
        bytes_left -= current_chunk_size;
        seq_num++;
        delay(20); // Small delay to ensure the receiver can process the packets
      }

      data_len = 0; // Reset data length after sending all data
    }
    else {
      // Serial.println("Invalid command received.");
      size_t ack_len = pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
      send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
    }
  }
  else { // command is not correct length
    // Serial.println("Invalid command length received.");
    size_t ack_len = pack_error(WRONG_CMD_CODE); // Pack error for invalid camera number
    send_reply(reply_ack_encoded, ack_len);      // Send acknowledgment reply
  }
}

size_t cobs_encode(const uint8_t* input, size_t length, uint8_t* output) {
  size_t  read_index  = 0;
  size_t  write_index = 1;
  size_t  code_index  = 0;
  uint8_t code        = 1;

  while (read_index < length) {
    if (input[read_index] == 0) {
      output[code_index] = code;
      code = 1;
      code_index = write_index++;
      read_index++;
    }
    else {
      output[write_index++] = input[read_index++];
      code++;

      if (code == 0xFF) {
        output[code_index] = code;
        code = 1;
        code_index = write_index++;
      }
    }
  }

  output[code_index] = code;

  return write_index;
}

size_t cobs_decode(const uint8_t* input, size_t length, uint8_t* output) {
  size_t read_index  = 0;
  size_t write_index = 0;
  uint8_t code       = 0;
  uint8_t i          = 0;

  while (read_index < length) {
    code = input[read_index];

    read_index++;

    for (i = 1; i < code; i++) { output[write_index++] = input[read_index++];}

    if (code != 0xFF && read_index != length) {output[write_index++] = '\0';}
  }

  return write_index;
}





