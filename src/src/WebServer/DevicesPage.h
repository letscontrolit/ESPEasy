#pragma once

#include "../WebServer/common.h"


#ifdef WEBSERVER_DEVICES

# include "../DataTypes/DeviceIndex.h"
# include "../DataTypes/TaskIndex.h"
# include "../DataTypes/PluginID.h"


# if FEATURE_I2C
void format_I2C_port_description(taskIndex_t x);

#  if FEATURE_I2CMULTIPLEXER
void ShowI2CMultiplexerUI(uint8_t i2cBus,
                          bool    muxPortsOption,
                          int     taskDeviceI2CMuxPort);
void GetI2CMultiplexerFromPage(uint8_t i2cBus,
                               bool  & muxPortsOption,
                               int   & selectedPorts);
#  endif // if FEATURE_I2CMULTIPLEXER
# endif // if FEATURE_I2C

void handle_devices();

#endif // ifdef WEBSERVER_DEVICES
