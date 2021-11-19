#include "../PluginStructs/P121_data_struct.h"

#ifdef USES_P121

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

bool P121_data_struct::begin()
{
  if (!initialized)
  {
    initialized = mag.begin();

    if (initialized)
    {
      // Set up oversampling and filter initialization
      sensor_t sensor;
      mag.getSensor(&sensor);
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
      String log = F("Sensor:       ");
      log += F(sensor.name);
      log += F("\nDriver Ver:   "); 
      log += sensor.version;
      log += F("\nUnique ID:    "); 
      log += sensor.sensor_id;
      log += F("\nMax Value:    "); 
      log += String(sensor.max_value);
      log += F("\nMin Value:    ");
      log += String(sensor.min_value);
      log += F("\nResolution:   "); 
      log += String(sensor.resolution);
      addLog(LOG_LEVEL_DEBUG, log);
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
      #endif
    }
  }

  return initialized;
}

#endif // ifdef USES_P121
