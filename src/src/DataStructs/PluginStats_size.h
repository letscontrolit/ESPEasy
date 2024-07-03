#ifndef HELPERS_PLUGINSTATS_SIZE_H
#define HELPERS_PLUGINSTATS_SIZE_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

# include <CircularBuffer.h>

# ifndef PLUGIN_STATS_NR_ELEMENTS
#  ifdef ESP8266
#   ifdef USE_SECOND_HEAP
#    define PLUGIN_STATS_NR_ELEMENTS 50
#   else // ifdef USE_SECOND_HEAP
#    define PLUGIN_STATS_NR_ELEMENTS 16
#   endif // ifdef USE_SECOND_HEAP
#  endif // ifdef ESP8266
#  ifdef ESP32
#   define PLUGIN_STATS_NR_ELEMENTS 250
#  endif // ifdef ESP32
# endif  // ifndef PLUGIN_STATS_NR_ELEMENTS


#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef HELPERS_PLUGINSTATS_SIZE_H
