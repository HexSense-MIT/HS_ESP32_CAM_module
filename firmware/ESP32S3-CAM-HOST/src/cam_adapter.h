#pragma once

#include <Arduino.h>
#include <SparkFun_I2C_Expander_Arduino_Library.h>

extern SFE_PCA95XX io; // Global instance of the PCA95XX I2C expander

extern void camadapter_init(void);
extern void turnoncam(int cam);
extern void turnoffcam(int cam);
extern void turnoffallcams(void);