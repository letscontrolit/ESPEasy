#ifndef PLUGINSTRUCTS_P013_DATA_STRUCT_H
#define PLUGINSTRUCTS_P013_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P013

# include <map>
# include <NewPing.h>

// Defines moved here to avoid Arduino-peculiar #if behavior

# ifdef ESP8266_1M // Exclude trigger-width feature on size-restricted builds
#  define P013_FEATURE_TRIGGERWIDTH   0
#  define P013_FEATURE_INTERVALEVENT  0
#  define P013_FEATURE_COMBINED_MODE  0
#  define P013_EXTENDED_LOG           0
# endif // ifdef ESP8266_1M
# ifndef P013_FEATURE_TRIGGERWIDTH
#  define P013_FEATURE_TRIGGERWIDTH   1 // Enabled by default
#  define P013_FEATURE_INTERVALEVENT  0
#  define P013_FEATURE_COMBINED_MODE  1
#  define P013_EXTENDED_LOG           1
# endif // ifndef P013_FEATURE_TRIGGERWIDTH

// PlugIn specific defines
// operatingMode
# define OPMODE_VALUE        (0)
# define OPMODE_STATE        (1)
# define OPMODE_COMBINED     (2)

// measuringUnit
# define UNIT_CM             (0)
# define UNIT_INCH           (1)

// filterType
# define FILTER_NONE         (0)
# define FILTER_MEDIAN       (1)

# define P013_DEFAULT_FILTER_SIZE     (5)
# define P013_DEFAULT_TRIGGER_WIDTH   (10)

# define P013_TRIGGER_PIN       CONFIG_PIN1
# define P013_ECHO_PIN          CONFIG_PIN2

# define P013_OPERATINGMODE     PCONFIG(0)
# define P013_THRESHOLD         PCONFIG(1)
# define P013_MAX_DISTANCE      PCONFIG(2)
# define P013_MEASURINGUNIT     PCONFIG(3)
# define P013_FILTERTYPE        PCONFIG(4)
# define P013_FILTER_SIZE       PCONFIG(5)
# if P013_FEATURE_TRIGGERWIDTH
#  define P013_TRIGGER_WIDTH     PCONFIG(6)
# else // if P013_FEATURE_TRIGGERWIDTH
#  define P013_TRIGGER_WIDTH     P013_DEFAULT_TRIGGER_WIDTH
# endif // if P013_FEATURE_TRIGGERWIDTH
# define P013_SEND_STATE_VALUE  PCONFIG(7)

#endif // ifdef USES_P013
#endif // ifndef PLUGINSTRUCTS_P013_DATA_STRUCT_H
