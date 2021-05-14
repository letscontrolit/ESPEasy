#ifndef PLUGINSTRUCTS_P115_DATA_STRUCT_H
#define PLUGINSTRUCTS_P115_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P115

# include <Wire.h> // Needed for I2C
// https://github.com/sparkfun/SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library
# include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

struct P115_data_struct : public PluginTaskData_base {
public:

  P115_data_struct(uint8_t                i2c_addr,
                   sfe_max1704x_devices_e device,
                   int                    threshold);

  bool begin();

  // Perform read and return true when an alert has been high
  bool read(bool clearAlert);

  void clearAlert();

  const sfe_max1704x_devices_e _device;
  SFE_MAX1704X                 lipo;
  int                          _threshold;

  float voltage    = 0.0f;  // LiPo voltage
  float soc        = 0.0f;  // LiPo state-of-charge (SOC)
  bool  alert      = false; // Whether alert has been triggered
  float changeRate = 0.0f;  // (MAX17048/49) Get rate of change per hour in %.
                            // A positive rate is charging, negative is discharge.
  bool initialized = false;
};


#endif // ifdef USES_P115
#endif // ifndef PLUGINSTRUCTS_P115_DATA_STRUCT_H
