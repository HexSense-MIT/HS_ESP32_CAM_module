#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_server.h"
#include "esp_camera.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "index.h"
#include "driver/temperature_sensor.h"
#include "driver/ledc.h"
#include "driver/i2c_master.h"

// Logging tag
static const char *TAG = "CAM_STREAM";

// Global variables for temperature and latest frame
static volatile float g_temperature = 0.0;
static volatile float g_fps = 0.0;

//********** UART Pin Definitions **********
#define UART_RX_PIN 1
#define UART_TX_PIN 2
#define TRIGGER_PIN 3

//********** Camera Pin Definitions **********
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 18
#define SIOD_GPIO_NUM 40
#define SIOC_GPIO_NUM 39
#define Y9_GPIO_NUM 48
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 16
#define Y6_GPIO_NUM 14
#define Y5_GPIO_NUM 12
#define Y4_GPIO_NUM 10
#define Y3_GPIO_NUM 11
#define Y2_GPIO_NUM 13
#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM 47
#define PCLK_GPIO_NUM 15

httpd_handle_t camera_httpd = NULL;
httpd_handle_t control_httpd = NULL;

uint8_t cmd_recv = 0; // Command received from UART
camera_fb_t *fb = NULL; // Frame buffer for camera

uint8_t IMAGE_TK_NCK[9]     =  {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t IMAGE_TK_ACK[9]     =  {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t IMAGE_TK_UNKNOWN[9] =  {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

//********** Camera Configuration **********
static camera_config_t camera_config = {
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sccb_sda   = SIOD_GPIO_NUM,
    .pin_sccb_scl   = SIOC_GPIO_NUM,
    
    .pin_d0         = Y2_GPIO_NUM,
    .pin_d1         = Y3_GPIO_NUM,
    .pin_d2         = Y4_GPIO_NUM,
    .pin_d3         = Y5_GPIO_NUM,
    .pin_d4         = Y6_GPIO_NUM,
    .pin_d5         = Y7_GPIO_NUM,
    .pin_d6         = Y8_GPIO_NUM,
    .pin_d7         = Y9_GPIO_NUM,

    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,

    .xclk_freq_hz   = 5000000,
    .ledc_timer     = LEDC_TIMER_0,
    .ledc_channel   = LEDC_CHANNEL_0,
    .pixel_format   = PIXFORMAT_JPEG,
    .frame_size     = FRAMESIZE_SXGA,//FRAMESIZE_HQVGA,//FRAMESIZE_QQVGA,
    .jpeg_quality   = 4, // seems like this is the best quality for the ESP32S3 without crashing
    .fb_count       = 1,
    .grab_mode      = CAMERA_GRAB_WHEN_EMPTY,
    .fb_location    = CAMERA_FB_IN_PSRAM,
};

// ********** UART Configuration **********
static const int RX_BUF_SIZE = 512;

void uart_init(void) {
  const uart_config_t uart_config = {
    .baud_rate = 250000,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };

  // We won't use a buffer for sending data.
  uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

//---------------------------------------------------------------------
// Temperature Endpoint Handler ("/temp")
// Returns the current temperature as JSON
//---------------------------------------------------------------------
int rx_cmd(uint8_t* data) {
  int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 10 / portTICK_PERIOD_MS);
  return rxBytes;
}

int tx_data(uint8_t* data, int len) {
  int txBytes = uart_write_bytes(UART_NUM_1, data, len);
  return txBytes;
}

static esp_err_t capture_handler(void) {
  if (fb) {
    esp_camera_fb_return(fb);
    fb = NULL; // Reset the frame buffer pointer
  }

  fb = esp_camera_fb_get();

  if (!fb) {
      ESP_LOGE(TAG, "Camera capture failed");
      tx_data(IMAGE_TK_NCK, sizeof(IMAGE_TK_NCK)); // Send negative acknowledgment
      return ESP_FAIL;
  }

  size_t image_size = fb->len;
  uint8_t* temp_byte = (uint8_t*)&image_size;
  memcpy(IMAGE_TK_ACK + 1, temp_byte, sizeof(size_t)); // Copy image size into ACK message

  // Print the image size
  printf("Image size: %zu bytes\n", fb->len);

  tx_data(IMAGE_TK_ACK, sizeof(IMAGE_TK_ACK)); // Send acknowledgment

  return ESP_OK;
}

static esp_err_t send_image_handler(void) {
  // Print the image data in hexadecimal format
  for (size_t i = 0; i < fb->len; ++i) {
    printf("%02X", fb->buf[i]);
    tx_data(&fb->buf[i], 1); // Send each byte over UART
    if ((i + 1) % 100 == 0) {
      vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
    }
  }
  printf("\n");

  esp_camera_fb_return(fb);
  fb = NULL;

  return ESP_OK;
}

//---------------------------------------------------------------------
// Main Application Entry Point
//---------------------------------------------------------------------
void app_main(void) {
  uart_init();

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  // Initialize camera
  ret = esp_camera_init(&camera_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed with error 0x%x", ret);
    return;
  }

  while (1) {
    if (rx_cmd(&cmd_recv) > 0) { // If command is received from UART
      if (cmd_recv == 0x01) { // Command to capture an image
        capture_handler();
      }
      else if (cmd_recv == 0x02) { // Command to send the image
        if (fb) {
          send_image_handler();
        } else {
          ESP_LOGE(TAG, "No image captured to send");
          tx_data(IMAGE_TK_NCK, sizeof(IMAGE_TK_NCK)); // Send negative acknowledgment
        }
      } else {
        ESP_LOGW(TAG, "Unknown command received: %02X", cmd_recv);
        tx_data(IMAGE_TK_UNKNOWN, sizeof(IMAGE_TK_UNKNOWN)); // Send unknown command response
      }
    }
  }
}




