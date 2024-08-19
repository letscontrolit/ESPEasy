#ifndef PLUGINSTRUCTS_P019_DATA_STRUCT_H
#define PLUGINSTRUCTS_P019_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P019

# include "../Helpers/_Plugin_Helper_GPIO.h"

/**************************************************\
   CONFIG
   TaskDevicePluginConfig settings:
   0: send boot state (true,false)
   1:
   2:
   3:
   4: use doubleclick (0,1,2,3)
   5: use longpress (0,1,2,3)
   6: LP fired (true,false)           20240818 GN: No longer used
   7: doubleclick counter (=0,1,2,3)  20240818 GN: No longer used

   TaskDevicePluginConfigFloat settings:
   0: debounce interval ms
   1: doubleclick interval ms
   2: longpress interval ms
   3: use safebutton (=0,1)

   TaskDevicePluginConfigLong settings:  (20240818 GN: No longer used)
   0: clickTime debounce ms
   1: clickTime doubleclick ms
   2: clickTime longpress ms
   3: safebutton counter (=0,1)
\**************************************************/


# define P019_BOOTSTATE     PCONFIG(0)
# define P019_DEBOUNCE      PCONFIG_FLOAT(0)
# define P019_DOUBLECLICK   PCONFIG(4)
# define P019_DC_MAX_INT    PCONFIG_FLOAT(1)
# define P019_LONGPRESS     PCONFIG(5)
# define P019_LP_MIN_INT    PCONFIG_FLOAT(2)
# define P019_SAFE_BTN      PCONFIG_FLOAT(3)


struct P019_data_struct : public PluginTaskData_base {
  static uint8_t getI2C_address(struct EventStruct *event);

  P019_data_struct() = delete;

  P019_data_struct(struct EventStruct *event);

  void tenPerSecond(struct EventStruct *event);

private:

  GPIO_plugin_helper_data_t _data;

# if FEATURE_I2C_DEVICE_CHECK
  const uint8_t _address;
# endif // if FEATURE_I2C_DEVICE_CHECK
};

#endif  // ifdef USES_P019
#endif  // ifndef PLUGINSTRUCTS_P019_DATA_STRUCT_H
