#ifndef PLUGINSTRUCTS_P074_DATA_STRUCT_H
#define PLUGINSTRUCTS_P074_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P074


# include <Adafruit_Sensor.h>
# include <Adafruit_TSL2591.h>


struct P074_data_struct : public PluginTaskData_base {
  P074_data_struct();
  virtual ~P074_data_struct() = default;

  // Changing the integration time gives you a longer time over which to sense
  // light
  // longer timelines are slower, but are good in very low light situtations!
  void setIntegrationTime(int time);

  // You can change the gain on the fly, to adapt to brighter/dimmer light
  // situations
  void setGain(int gain);

  // Return true when value is present.
  bool getFullLuminosity(uint32_t& value);

  Adafruit_TSL2591 tsl;
  unsigned long    integrationStart       = 0;
  unsigned long    duration               = 0;
  bool             integrationActive      = false;
  bool             newValuePresent        = false;
  bool             startIntegrationNeeded = false;
};


#endif // ifdef USES_P074
#endif // ifndef PLUGINSTRUCTS_P074_DATA_STRUCT_H
