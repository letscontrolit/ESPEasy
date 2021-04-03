#ifndef PLUGINSTRUCTS_P112_DATA_STRUCT_H
#define PLUGINSTRUCTS_P112_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P112

// #######################################################################################################
// #################### Plugin 112 I2C AS7265X Triad Spectroscopy Sensor and White, IR and UV LED ########
// #######################################################################################################
//
// Triad Spectroscopy Sensor and White, IR and UV LED
// like this one: https://www.sparkfun.com/products/15050
// based on this library: https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library
// this code is based on 29 Mar 2019-03-29 version of the above library
//
// 2021-03-29 heinemannj: Initial commit
//

#include <SparkFun_AS7265X.h>

struct P112_data_struct : public PluginTaskData_base {
  bool begin();
  AS7265X sensor;
  bool initialized = false;

  // MeasurementStatus:
  // 0      : Not running
  // 1 - 18 : Running
  byte MeasurementStatus = 0;
};

#endif // ifdef USES_P112
#endif // ifndef PLUGINSTRUCTS_P112_DATA_STRUCT_H
