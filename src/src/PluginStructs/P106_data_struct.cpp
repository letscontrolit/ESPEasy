#include "../PluginStructs/P106_data_struct.h"

#ifdef USES_P106


// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>

P106_data_struct::~P106_data_struct()
{
  if (bme != nullptr) {
    delete bme;
    bme = nullptr;
  }

}

bool P106_data_struct::begin(uint8_t addr, bool initSettings)
{
  if (!initialized) {
    if (bme == nullptr) {
      bme = new (std::nothrow) Adafruit_BME680();
    }
    if (bme == nullptr) {
      return false;
    }
    initialized = bme->begin(addr, initSettings);

    if (initialized) {
      // Set up oversampling and filter initialization
      bme->setTemperatureOversampling(BME68X_OS_8X);
      bme->setHumidityOversampling(BME68X_OS_2X);
      bme->setPressureOversampling(BME68X_OS_4X);
      bme->setIIRFilterSize(BME68X_FILTER_SIZE_3);
      bme->setGasHeater(320, 150); // 320*C for 150 ms
    }
  }

  return initialized;
}

  bool P106_data_struct::performReading()
  {
    if (bme == nullptr) return false;
    return bme->performReading();
  }

  float P106_data_struct::getTemperature() const
  {
    if (bme == nullptr) return 0.0f;
    return bme->temperature;
  }

  float P106_data_struct::getHumidity() const
  {
    if (bme == nullptr) return 0.0f;
    return bme->humidity;
  }


  float P106_data_struct::getPressure() const
  {
    if (bme == nullptr) return 0.0f;
    return bme->pressure / 100.0f;
  }


  float P106_data_struct::getGasResistance() const
  {
    if (bme == nullptr) return 0.0f;
    return bme->gas_resistance;
  }



#endif // ifdef USES_P106
