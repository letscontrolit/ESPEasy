#include "../PluginStructs/P121_data_struct.h"

#ifdef USES_P121

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
# include <Adafruit_Sensor.h>
# include <Adafruit_HMC5883_U.h>

bool P121_data_struct::begin(int taskid)
{
  if (!initialized) {
    mag         = Adafruit_HMC5883_Unified(taskid);
    initialized = mag.begin();

    if (initialized) {
      // Set up oversampling and filter initialization
      sensor_t sensor;
      mag.getSensor(&sensor);
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLogMove(LOG_LEVEL_DEBUG,
                   strformat(F("HMC5883_U: Sensor: %s, Driver Ver: %d, Unique ID: %d, Max Value: %.2f, Min Value: %.2f, Resolution: %.2f"),
                             sensor.name,
                             sensor.version,
                             sensor.sensor_id,
                             sensor.max_value,
                             sensor.min_value,
                             sensor.resolution));
      }
      # endif // ifndef BUILD_NO_DEBUG
    }
  }

  return initialized;
}

#endif // ifdef USES_P121
