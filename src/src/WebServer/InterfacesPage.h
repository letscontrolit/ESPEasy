#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_INTERFACES

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_interfaces();

# if FEATURE_PLUGIN_PRIORITY
bool isI2CPriorityTaskActive(uint8_t i2cBus);
void I2CShowSdaSclReadonly(int8_t  i2c_sda,
                           int8_t  i2c_scl,
                           uint8_t i2cBus);
# endif
#endif 
