#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_INTERFACES

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_interfaces();

void save_interfaces();
#if FEATURE_I2C
void handle_interfaces_i2c();
bool save_I2C(String& error);
void interfaces_show_I2C();
#endif
#if FEATURE_SPI
void handle_interfaces_spi();
bool save_SPI(String& error);
void interfaces_show_SPI();
#endif

#if defined(FEATURE_MODBUS) && FEATURE_MODBUS
void handle_interfaces_modbus();
bool save_MODBUS(String& error);
void interfaces_show_MODBUS();
#endif // if defined(FEATURE_MODBUS) && FEATURE_MODBUS

#if defined(FEATURE_CAN) && FEATURE_CAN
void handle_interfaces_can();
bool save_CAN(String& error);
void interfaces_show_CAN();
#endif // if defined(FEATURE_CAN) && FEATURE_CAN

#if defined(FEATURE_WRMBUS) && FEATURE_WRMBUS
void handle_interfaces_wrmbus();
bool save_WRMBUS(String& error);
void interfaces_show_WRMBUS();
#endif // if defined(FEATURE_WRMBUS) && FEATURE_WRMBUS

#if defined(FEATURE_WIMBUS) && FEATURE_WIMBUS
void handle_interfaces_wimbus();
bool save_WIMBUS(String& error);
void interfaces_show_WIMBUS();
#endif // if defined(FEATURE_WIMBUS) && FEATURE_WIMBUS


#if FEATURE_I2C
# if FEATURE_PLUGIN_PRIORITY
bool isI2CPriorityTaskActive(uint8_t i2cBus);
void I2CShowSdaSclReadonly(int8_t  i2c_sda,
                           int8_t  i2c_scl,
                           uint8_t i2cBus);
# endif
#endif
#endif 
