#ifndef PLUGINSTRUCTS_P117_DATA_STRUCT_H
#define PLUGINSTRUCTS_P117_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P117
# include <FrogmoreScd30.h>

struct P117_data_struct : public PluginTaskData_base {
public:

  P117_data_struct(uint16_t altitude,
                   float    temperatureOffset,
                   bool     autoCalibration);

  P117_data_struct() = delete;

  uint32_t read_sensor(uint16_t *scd30_CO2,
                       uint16_t *scd30_CO2EAvg,
                       float    *scd30_Temp,
                       float    *scd30_Humid);
  bool softReset();
  void getCalibrationType(uint16_t *abc) {
    if (initialised) {
      scd30.getCalibrationType(abc);
    }
  }

  void getAltitudeCompensation(uint16_t *altitude) {
    if (initialised) {
      scd30.getAltitudeCompensation(altitude);
    }
  }

  void getTemperatureOffset(float *temperature) {
    if (initialised) {
      scd30.getTemperatureOffset(temperature);
    }
  }

  int setCalibrationMode(bool isAuto);
  int setForcedRecalibrationFactor(uint16_t co2_ppm);

private:

  FrogmoreScd30 scd30;

  bool init_sensor();

  uint16_t _altitude;
  float    _temperatureOffset;
  bool     _autoCalibration;

  bool initialised = false;
};

#endif // ifdef USES_P117
#endif // ifndef PLUGINSTRUCTS_P117_DATA_STRUCT_H
