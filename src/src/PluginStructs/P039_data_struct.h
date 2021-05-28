#ifndef PLUGINSTRUCTS_P039_DATA_STRUCT_H
#define PLUGINSTRUCTS_P039_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P039


struct P039_data_struct : public PluginTaskData_base {
public:

  P039_data_struct(uint16_t               conversionResult,
                   uint8_t                deviceFaults,
                   unsigned long          timer,
                   bool                   sensorFault,
                   bool                   convReady);

  P039_data_struct() = default;

  bool begin();

  // Perform read and return true when an alert has been high
  bool read();

  // Perform write and return true when an alert has been high
  bool write();

  // uint8_t mainState = 0x00u;;
  // uint8_t command = 0x00u;
  uint16_t conversionResult = 0x0000u;
  uint8_t deviceFaults = 0x00u;
  unsigned long  timer = 0;
  bool sensorFault = false;
  bool convReady = false;

};


#endif // ifdef USES_P039
#endif // ifndef PLUGINSTRUCTS_P039_DATA_STRUCT_H