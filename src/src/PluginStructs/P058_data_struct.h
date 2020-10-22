#ifndef PLUGINSTRUCTS_P058_DATA_STRUCT_H
#define PLUGINSTRUCTS_P058_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P058

#include "../../ESPEasy_common.h"

# include <HT16K33.h>


struct P058_data_struct : public PluginTaskData_base {
public:

  P058_data_struct(uint8_t i2c_addr);

  // Read key
  // @retval True when key has changed since last check.
  bool readKey(uint8_t& key);

private:

  CHT16K33 keyPad;
  uint8_t  keyLast = 0;
};

#endif // ifdef USES_P058
#endif // ifndef PLUGINSTRUCTS_P058_DATA_STRUCT_H
