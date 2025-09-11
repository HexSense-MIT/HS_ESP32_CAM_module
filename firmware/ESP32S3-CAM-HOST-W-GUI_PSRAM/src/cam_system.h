#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp_heap_caps.h>

#include "cam_adapter.h"

#define CAM_RX_PIN 13
#define CAM_TX_PIN 12

extern void system_init(void);


