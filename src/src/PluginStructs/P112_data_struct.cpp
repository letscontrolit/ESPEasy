#include "../PluginStructs/P112_data_struct.h"

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

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <SparkFun_AS7265X.h>

bool P112_data_struct::begin()
{
  if (!initialized) {
    initialized = sensor.begin();

    if (initialized) {
//      sensor.takeMeasurementsWithBulb();
    }
  }
  return initialized;
}

#endif // ifdef USES_P112
