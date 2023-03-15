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
        String log = F("HMC5883_U: Sensor: ");
        log += String(sensor.name);
        log += F(", Driver Ver: ");
        log += sensor.version;
        log += F(", Unique ID: ");
        log += sensor.sensor_id;
        log += F(", Max Value: ");
        log += toString(sensor.max_value);
        log += F(", Min Value: ");
        log += toString(sensor.min_value);
        log += F(", Resolution: ");
        log += toString(sensor.resolution);
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifndef BUILD_NO_DEBUG
    }
  }

  return initialized;
}

#endif // ifdef USES_P121
