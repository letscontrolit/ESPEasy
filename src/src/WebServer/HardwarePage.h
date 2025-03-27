#ifndef WEBSERVER_WEBSERVER_HARDWAREPAGE_H
#define WEBSERVER_WEBSERVER_HARDWAREPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_HARDWARE

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_hardware();

# if FEATURE_PLUGIN_PRIORITY
bool isI2CPriorityTaskActive(uint8_t i2cBus);
void I2CShowSdaSclReadonly(int8_t  i2c_sda,
                           int8_t  i2c_scl,
                           uint8_t i2cBus);
# endif // if FEATURE_PLUGIN_PRIORITY
#endif // ifdef WEBSERVER_HARDWARE

#endif // ifndef WEBSERVER_WEBSERVER_HARDWAREPAGE_H
