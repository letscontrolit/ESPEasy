#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_INTERFACES

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_interfaces();

# if FEATURE_I2C
void handle_interfaces_i2c();
# endif
# if FEATURE_SPI
void handle_interfaces_spi();
# endif

# if FEATURE_MODBUS && FEATURE_MODBUS_INTERFACES_TAB
void handle_interfaces_modbus();
# endif // if FEATURE_MODBUS

# if FEATURE_CAN
void handle_interfaces_can();
# endif // if FEATURE_CAN

# if FEATURE_WRMBUS
void handle_interfaces_wrmbus();
# endif // if FEATURE_WRMBUS

# if FEATURE_WIMBUS
void handle_interfaces_wimbus();
# endif // if FEATURE_WIMBUS

#endif // ifdef WEBSERVER_INTERFACES
