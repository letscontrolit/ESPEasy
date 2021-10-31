#ifndef PLUGINSTRUCTS_P120_DATA_STRUCT_H
#define PLUGINSTRUCTS_P120_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P120

# include <Wire.h> // Needed for I2C
// https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
# include <SparkFun_ADXL345.h>

struct P120_data_struct : public PluginTaskData_base {
public:

  P120_data_struct(uint8_t                i2c_addr,
                   int                    threshold);

  // bool begin();

  // // Perform read and return true when an alert has been high
  // bool read(bool clearAlert);

  // void clearAlert();

  int                          _threshold;

  bool initialized = false;
};


#endif // ifdef USES_P120
#endif // ifndef PLUGINSTRUCTS_P120_DATA_STRUCT_H
