#include "../PluginStructs/P117_data_struct.h"

#ifdef USES_P117

// **************************************************************************/
// Constructor
// **************************************************************************/
P117_data_struct::P117_data_struct(uint16_t altitude,
                                   float    temperatureOffset,
                                   bool     autoCalibration,
                                   uint16_t interval)
  : _altitude(altitude), _temperatureOffset(temperatureOffset), _autoCalibration(autoCalibration), _interval(interval) {}


// **************************************************************************/
// Initialize sensor and read data from SCD30
// **************************************************************************/
uint32_t P117_data_struct::read_sensor(uint16_t *scd30_CO2, uint16_t *scd30_CO2EAvg, float *scd30_Temp, float *scd30_Humid) {
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

    scd30.setCalibrationType(_autoCalibration);
    scd30.setMeasurementInterval(_interval);

    scd30.beginMeasuring();
    scd30.setAltitudeCompensation(_altitude);
    scd30.setTemperatureOffset(_temperatureOffset);
    return true;
  }

  return initialised;
}

int P117_data_struct::setCalibrationMode(bool isAuto) {
  if (initialised) {
    _autoCalibration = isAuto;
    return scd30.setCalibrationType(isAuto);
  }
  return ERROR_SCD30_NOT_FOUND_ERROR;
}

int P117_data_struct::setForcedRecalibrationFactor(uint16_t co2_ppm) {
  if (initialised) {
    setCalibrationMode(false); // Force to manual mode
    return scd30.setForcedRecalibrationFactor(co2_ppm);
  }
  return ERROR_SCD30_NOT_FOUND_ERROR;
}

int P117_data_struct::setMeasurementInterval(uint16_t interval) {
  if (initialised) {
    if ((interval >= 2) && (interval <= 1800)) {
      return scd30.setMeasurementInterval(interval);
    } else {
      return ERROR_SCD30_INVALID_VALUE;
    }
  }
  return ERROR_SCD30_NOT_FOUND_ERROR;
}

#endif // ifdef USES_P117
