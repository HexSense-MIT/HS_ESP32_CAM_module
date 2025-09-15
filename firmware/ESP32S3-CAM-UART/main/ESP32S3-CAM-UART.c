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

//---------------------------------------------------------------------
// Temperature Endpoint Handler ("/temp")
// Returns the current temperature as JSON
//---------------------------------------------------------------------

static esp_err_t capture_handler(void) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
      ESP_LOGE(TAG, "Camera capture failed");
      return ESP_FAIL;
  }

  // If the image is not in JPEG format, convert it.
  if(fb->format != PIXFORMAT_JPEG) {
      size_t out_len = 0;
      uint8_t *out_buf = NULL;
      bool jpg_converted = frame2jpg(fb, 80, &out_buf, &out_len);
      if(!jpg_converted) {
          ESP_LOGE(TAG, "JPEG compression failed");
          esp_camera_fb_return(fb);
          return ESP_FAIL;
      }
      free(out_buf);
  } else {
    // Print the image size
    printf("Image size: %zu bytes\n", fb->len);

    // Print the image data in hexadecimal format
    for (size_t i = 0; i < fb->len; ++i) {
      printf("%02X", fb->buf[i]);
      if ((i + 1) % 100 == 0) {
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
      }
    }
    printf("\n");
  }
  esp_camera_fb_return(fb);
  return ESP_OK;
}

//---------------------------------------------------------------------
// Main Application Entry Point
//---------------------------------------------------------------------
void app_main(void) {
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
    capture_handler();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}