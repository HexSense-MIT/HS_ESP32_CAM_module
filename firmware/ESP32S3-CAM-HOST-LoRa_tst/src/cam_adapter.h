#pragma once

#include <Arduino.h>
#include <SparkFun_I2C_Expander_Arduino_Library.h>

extern SFE_PCA95XX io; // Global instance of the PCA95XX I2C expander

extern void camadapter_init(void);
extern void turnoncam(uint8_t cam);
extern void turnoffcam(uint8_t cam);
extern void turnoffallcams(void);

extern void flush_buffer(void);