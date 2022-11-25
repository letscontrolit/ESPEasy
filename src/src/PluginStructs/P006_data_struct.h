#ifndef PLUGINSTRUCTS_P006_DATA_STRUCT_H
#define PLUGINSTRUCTS_P006_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P006


# define BMP085_ULTRAHIGHRES         3

struct P006_data_struct : public PluginTaskData_base {
  P006_data_struct() = default;
  virtual ~P006_data_struct() = default;

  bool     begin();

  uint16_t readRawTemperature();

  uint32_t readRawPressure();

  int32_t  readPressure();

  float    readTemperature();

  uint8_t  oversampling = BMP085_ULTRAHIGHRES;
  int16_t  ac1 = 0;
  int16_t  ac2 = 0;
  int16_t  ac3 = 0;
  int16_t  b1 = 0;
  int16_t  b2 = 0;
  int16_t  mb = 0;
  int16_t  mc = 0;
  int16_t  md = 0;
  uint16_t ac4 = 0;
  uint16_t ac5 = 0;
  uint16_t ac6 = 0;

  bool initialized = false;
};

#endif // ifdef USES_P006
#endif // ifndef PLUGINSTRUCTS_P006_DATA_STRUCT_H
