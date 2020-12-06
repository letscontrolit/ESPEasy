#ifndef PLUGINSTRUCTS_P006_DATA_STRUCT_H
#define PLUGINSTRUCTS_P006_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P006


# define BMP085_ULTRAHIGHRES         3

struct P006_data_struct : public PluginTaskData_base {
  bool     begin();

  uint16_t readRawTemperature(void);

  uint32_t readRawPressure(void);

  int32_t  readPressure(void);

  float    readTemperature(void);

  float    pressureElevation(float atmospheric,
                             int   altitude);

  uint8_t  oversampling = BMP085_ULTRAHIGHRES;
  int16_t  ac1, ac2, ac3, b1, b2, mb, mc, md = 0;
  uint16_t ac4, ac5, ac6 = 0;

  bool initialized = false;
};

#endif // ifdef USES_P006
#endif // ifndef PLUGINSTRUCTS_P006_DATA_STRUCT_H
