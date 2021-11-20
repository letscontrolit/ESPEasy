#include "../PluginStructs/P117_data_struct.h"

#ifdef USES_P117

// **************************************************************************/
// Constructor
// **************************************************************************/
P117_data_struct::P117_data_struct(uint16_t altitude, float temperatureOffset)
  : _altitude(altitude), _temperatureOffset(temperatureOffset) {}


// **************************************************************************/
// Initialize sensor and read data from SCD30
// **************************************************************************/
int P117_data_struct::read_sensor(uint16_t *scd30_CO2, uint16_t *scd30_CO2EAvg, float *scd30_Temp, float *scd30_Humid) {
  if (!initialised) {
    initialised = init_sensor(); // Check id device is present
  }

  if (initialised) {
    return scd30.readMeasurement(scd30_CO2, scd30_CO2EAvg, scd30_Temp, scd30_Humid);
  }
  return ERROR_SCD30_NO_DATA;
}

// **************************************************************************/
// Check SCD30 presence and initialize
// **************************************************************************/
bool P117_data_struct::softReset() {
  if (initialised) {
    scd30.softReset();
  }
  return initialised;
}

// **************************************************************************/
// Check SCD30 presence and initialize
// **************************************************************************/
bool P117_data_struct::init_sensor() {
  if (!initialised) {
    scd30.begin();
    uint16_t calibration = 0;
    scd30.getCalibrationType(&calibration);

    if (calibration) {
      scd30.setManualCalibration();
    }
    scd30.beginMeasuring();
    scd30.setAltitudeCompensation(_altitude);
    scd30.setTemperatureOffset(_temperatureOffset);
    return true;
  }

  return initialised;
}

#endif // ifdef USES_P117
