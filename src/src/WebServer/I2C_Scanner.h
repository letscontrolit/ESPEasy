#ifndef WEBSERVER_WEBSERVER_I2C_SCANNER_H
#define WEBSERVER_WEBSERVER_I2C_SCANNER_H

#include "../WebServer/common.h"



#ifdef WEBSERVER_I2C_SCANNER

#if FEATURE_I2CMULTIPLEXER
#include <vector>
typedef std::vector<bool> i2c_addresses_t;
#endif // if FEATURE_I2CMULTIPLEXER

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface I2C scanner
// ********************************************************************************

int scanI2CbusForDevices_json( // Utility function for scanning the I2C bus for valid devices, with JSON output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
      #if FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
      #endif // if FEATURE_I2CMULTIPLEXER
);

void handle_i2cscanner_json();
#endif // WEBSERVER_NEW_UI


String getKnownI2Cdevice(uint8_t address);

int scanI2CbusForDevices( // Utility function for scanning the I2C bus for valid devices, with HTML table output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
      #if FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
      #endif // if FEATURE_I2CMULTIPLEXER
);

// FIXME TD-er: Query all included plugins for their supported addresses (return name of plugin)
void handle_i2cscanner();
#endif // WEBSERVER_I2C_SCANNER




#endif