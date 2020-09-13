#ifndef PLUGINSTRUCTS_P062_DATA_STRUCT_H
#define PLUGINSTRUCTS_P062_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P062

# include <Adafruit_MPR121.h>


struct P062_data_struct : public PluginTaskData_base {
public:

  P062_data_struct(uint8_t i2c_addr,
                   bool    scancode);

  bool readKey(uint16_t& key);

private:

  Adafruit_MPR121 keypad;
  uint16_t        keyLast = 0;
  bool            use_scancode;
};

#endif // ifdef USES_P062
#endif // ifndef PLUGINSTRUCTS_P062_DATA_STRUCT_H
