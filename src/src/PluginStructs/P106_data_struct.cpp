#include "../PluginStructs/P106_data_struct.h"

#ifdef USES_P106


// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>


bool P106_data_struct::begin(uint8_t addr, bool initSettings)
{
  if (!initialized) {
    initialized = bme.begin(addr, initSettings);

    if (initialized) {
      // Set up oversampling and filter initialization
      bme.setTemperatureOversampling(BME680_OS_8X);
      bme.setHumidityOversampling(BME680_OS_2X);
      bme.setPressureOversampling(BME680_OS_4X);
      bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
      bme.setGasHeater(320, 150); // 320*C for 150 ms
    }
  }

  return initialized;
}

#endif // ifdef USES_P106
