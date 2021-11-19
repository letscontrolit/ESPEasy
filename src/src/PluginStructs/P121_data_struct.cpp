#include "../PluginStructs/P121_data_struct.h"

#ifdef USES_P121

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

bool P121_data_struct::begin(bool initSettings)
{
  if (!initialized)
  {
    initialized = mag.begin();

    if (initialized)
    {
      // Set up oversampling and filter initialization
      sensor_t sensor;
      mag.getSensor(&sensor);
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
      String log = F("Sensor:       ");
      log += sensor.name;
      log += "\nDriver Ver:   " + sensor.version;
      log += "\nUnique ID:    " + sensor.sensor_id;
      log += "\nMax Value:    " + String(sensor.max_value);
      log += "\nMin Value:    " + String(sensor.min_value);
      log += "\nResolution:   " + String(sensor.resolution);
      addLog(LOG_LEVEL_DEBUG, log);
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
    }
  }

  return initialized;
}

#endif // ifdef USES_P121
