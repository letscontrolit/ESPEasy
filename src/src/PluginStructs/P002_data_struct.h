#ifndef PLUGINSTRUCTS_P002_DATA_STRUCT_H
#define PLUGINSTRUCTS_P002_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P002

# ifdef ESP32
  #  define P002_MAX_ADC_VALUE    4095
# endif // ifdef ESP32
# ifdef ESP8266
  #  define P002_MAX_ADC_VALUE    1023
# endif // ifdef ESP8266


struct P002_data_struct : public PluginTaskData_base {
  P002_data_struct() = default;

  ~P002_data_struct();

  void reset();

  void addOversamplingValue(int currentValue);

  bool getOversamplingValue(float& float_value,
                            int  & raw_value) const;

  uint16_t OversamplingCount = 0;

private:

  int32_t OversamplingValue  = 0;
  int16_t OversamplingMinVal = P002_MAX_ADC_VALUE;
  int16_t OversamplingMaxVal = 0;
};


#endif // ifdef USES_P002
#endif // ifndef PLUGINSTRUCTS_P002_DATA_STRUCT_H
