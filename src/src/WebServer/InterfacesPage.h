#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_INTERFACES

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_interfaces();

void save_interfaces();
#if FEATURE_I2C
bool save_I2C(String& error);
void interfaces_show_I2C();
#endif
#if FEATURE_SPI
bool save_SPI(String& error);
void interfaces_show_SPI();
#endif

#if FEATURE_I2C
# if FEATURE_PLUGIN_PRIORITY
bool isI2CPriorityTaskActive(uint8_t i2cBus);
void I2CShowSdaSclReadonly(int8_t  i2c_sda,
                           int8_t  i2c_scl,
                           uint8_t i2cBus);
# endif
#endif
#endif 
