#ifndef PLUGINSTRUCTS_P001_DATA_STRUCT_H
#define PLUGINSTRUCTS_P001_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P001

# include "../Helpers/_Plugin_Helper_GPIO.h"

/**************************************************\
   CONFIG
   TaskDevicePluginConfig settings:
   0: button type (switch or dimmer)
   1: dim value
   2: button option (normal, push high, push low)
   3: send boot state (true,false)
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

// N.B. PCONFIG(0) ... PCONFIG(3) differ in P001 compared to P009 and P019

// Make sure the initial default is a switch (value 0)
# define PLUGIN_001_TYPE_SWITCH                   0
# define PLUGIN_001_TYPE_DIMMER                   3 // Due to some changes in previous versions, do not use 2.

# define P001_SWITCH_OR_DIMMER PCONFIG(0)  
# define P001_DIMMER_VALUE     PCONFIG(1)
# define P001_BUTTON_TYPE      PCONFIG(2) // (normal, push high, push low)

# define P001_BOOTSTATE        PCONFIG(3)
# define P001_DEBOUNCE         PCONFIG_FLOAT(0)
# define P001_DOUBLECLICK      PCONFIG(4)
# define P001_DC_MAX_INT       PCONFIG_FLOAT(1)
# define P001_LONGPRESS        PCONFIG(5)
# define P001_LP_MIN_INT       PCONFIG_FLOAT(2)
# define P001_SAFE_BTN         PCONFIG_FLOAT(3)


struct P001_data_struct : public PluginTaskData_base {
  static uint8_t P001_getSwitchType(struct EventStruct *event);

  P001_data_struct() = delete;

  P001_data_struct(struct EventStruct *event);

  void tenPerSecond(struct EventStruct *event);

private:

  GPIO_plugin_helper_data_t _data;
};

#endif // ifdef USES_P001
#endif // ifndef PLUGINSTRUCTS_P001_DATA_STRUCT_H
