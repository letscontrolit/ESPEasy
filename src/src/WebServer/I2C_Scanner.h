#ifndef WEBSERVER_WEBSERVER_I2C_SCANNER_H
#define WEBSERVER_WEBSERVER_I2C_SCANNER_H

#include "../WebServer/common.h"



#ifdef WEBSERVER_I2C_SCANNER

#ifdef FEATURE_I2CMULTIPLEXER
typedef std::vector<bool> i2c_addresses_t;
#endif

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface I2C scanner
// ********************************************************************************

int scanI2CbusForDevices_json( // Utility function for scanning the I2C bus for valid devices, with JSON output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
#ifdef FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
#endif
);

void handle_i2cscanner_json();
#endif // WEBSERVER_NEW_UI


String getKnownI2Cdevice(byte address);

int scanI2CbusForDevices( // Utility function for scanning the I2C bus for valid devices, with HTML table output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
#ifdef FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
#endif
);

// FIXME TD-er: Query all included plugins for their supported addresses (return name of plugin)
void handle_i2cscanner();
#endif // WEBSERVER_I2C_SCANNER




#endif