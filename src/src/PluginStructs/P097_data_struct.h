#ifndef PLUGINSTRUCTS_P097_DATA_STRUCT_H
#define PLUGINSTRUCTS_P097_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P097

# if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED

#  define LAST_TOUCH_INPUT_INDEX SOC_TOUCH_SENSOR_NUM

// Device-specific configuration
#  if defined(ESP32_CLASSIC)
  #   define HAS_T0_INPUT                 1
  #   define HAS_T10_TO_T14               0
  #   ifdef LAST_TOUCH_INPUT_INDEX
  #    undef LAST_TOUCH_INPUT_INDEX
  #   endif // ifdef LAST_TOUCH_INPUT_INDEX
  #   define LAST_TOUCH_INPUT_INDEX       10
  #   define P097_MAX_THRESHOLD_VALUE     4095
  #   define P097_DEFAULT_TOUCH_THRESHOLD 20
#  elif defined(ESP32S2) || defined(ESP32S3)
  #   define HAS_T0_INPUT                 0
  #   define HAS_T10_TO_T14               0 // Temporary disabled since T10 to T14 are causing problems
  #   ifdef LAST_TOUCH_INPUT_INDEX
  #    undef LAST_TOUCH_INPUT_INDEX
  #   endif // ifdef LAST_TOUCH_INPUT_INDEX
  #   define LAST_TOUCH_INPUT_INDEX       14
  #   define P097_MAX_THRESHOLD_VALUE     500000 // couldn't find a max value but threshold for ESP32S2 & ESP32S3 is uint32_t
  #   define P097_DEFAULT_TOUCH_THRESHOLD 1500
#  endif // if defined(ESP32_CLASSIC)

#  define P097_MAX_LONGPRESS_VALUE   10000

#  define P097_SEND_TOUCH_EVENT      PCONFIG(0)
#  define P097_SEND_RELEASE_EVENT    PCONFIG(1)
#  define P097_SEND_DURATION_EVENT   PCONFIG(2)
#  define P097_TOUCH_THRESHOLD       PCONFIG(3)
#  define P097_TYPE_TOGGLE           PCONFIG(4)
#  define P097_SLEEP_WAKEUP          PCONFIG(5)
#  define P097_SEND_LONG_PRESS_EVENT PCONFIG(6)
#  define P097_LONG_PRESS_TIME       PCONFIG(7)

# endif // if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED

#endif // ifdef USES_P097
#endif // ifndef PLUGINSTRUCTS_P097_DATA_STRUCT_H
