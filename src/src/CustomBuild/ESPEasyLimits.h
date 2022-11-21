#ifndef CUSTOMBUILD_ESPEASY_LIMITS_H
#define CUSTOMBUILD_ESPEASY_LIMITS_H

#include "../../include/ESPEasy_config.h"

// ***********************************************************************
// * These limits have direct impact on the settings files
// * Do not change them!
// * Else settings files will no longer be compatible with official builds
// * Some of these are related to the values defined in StorageLayout.h
// ***********************************************************************


// Performing a 2-stage define assignment using the _TMP defines
// See: https://github.com/letscontrolit/ESPEasy/issues/2621
#if FEATURE_NON_STANDARD_24_TASKS
  #define TASKS_MAX_TMP                      24
#else
  #define TASKS_MAX_TMP                      12
#endif


#if defined(ESP8266)
  #ifndef TASKS_MAX
    #define TASKS_MAX                          TASKS_MAX_TMP
  #endif
  #ifndef MAX_GPIO
    #define MAX_GPIO                           16
  #endif
#endif
#if defined(ESP32)
  #ifndef TASKS_MAX
    #define TASKS_MAX                          32
  #endif

  #ifndef MAX_GPIO
    #if ESP_IDF_VERSION_MAJOR > 3       // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32S2     // ESP32-S2
        #define MAX_GPIO  46
      #elif CONFIG_IDF_TARGET_ESP32C3   // ESP32-C3
        // FIXME TD-er: Implement ESP32C3 support
      #elif CONFIG_IDF_TARGET_ESP32     // ESP32/PICO-D4
        #define MAX_GPIO  39
      #else
        #error Target CONFIG_IDF_TARGET is not supported
      #endif
    #else // ESP32 Before IDF 4.0
      #define MAX_GPIO  39
    #endif
  #endif

#endif

#ifndef CONTROLLER_MAX
  #define CONTROLLER_MAX                      3 // max 4!
#endif
#ifndef NOTIFICATION_MAX
  #define NOTIFICATION_MAX                    3 // max 4!
#endif
#ifndef VARS_PER_TASK
  #define VARS_PER_TASK                       4
#endif
#ifndef PLUGIN_CONFIGVAR_MAX
  #define PLUGIN_CONFIGVAR_MAX                8
#endif
#ifndef PLUGIN_CONFIGFLOATVAR_MAX
  #define PLUGIN_CONFIGFLOATVAR_MAX           4
#endif
#ifndef PLUGIN_CONFIGLONGVAR_MAX
  #define PLUGIN_CONFIGLONGVAR_MAX            4
#endif
#ifndef PLUGIN_EXTRACONFIGVAR_MAX
  #define PLUGIN_EXTRACONFIGVAR_MAX          16
#endif
#ifndef NAME_FORMULA_LENGTH_MAX
  #define NAME_FORMULA_LENGTH_MAX            40
#endif

#define USERVAR_MAX_INDEX    (VARS_PER_TASK * TASKS_MAX)

// ***********************************************************************
// * The next limits affect memory usage
// ***********************************************************************
#ifndef DEVICES_MAX
  // TODO TD-er: This should be set automatically by counting the number of included plugins.
  # ifdef ESP32
    # define DEVICES_MAX                      140
  #else
    #if defined(PLUGIN_BUILD_COLLECTION) || defined(PLUGIN_BUILD_DEV)
      #  define DEVICES_MAX                      95
    # else 
      #  define DEVICES_MAX                      60
    # endif
  #endif
#endif

#ifndef PLUGIN_MAX
  #define PLUGIN_MAX                DEVICES_MAX
#endif
#ifndef CPLUGIN_MAX
  #define CPLUGIN_MAX                        20
#endif
#ifndef NPLUGIN_MAX
  #define NPLUGIN_MAX                         4
#endif

#ifndef UNIT_NUMBER_MAX
  #define UNIT_NUMBER_MAX                   254 // Stored in Settings.Unit  unit 255 = broadcast
#endif


// ***********************************************************************
// * Limits regarding Rules
// ***********************************************************************

#ifndef RULES_TIMER_MAX
  #define RULES_TIMER_MAX                   256
#endif
//#ifndef PINSTATE_TABLE_MAX
//#define PINSTATE_TABLE_MAX                 32
//#endif
#ifndef RULES_MAX_SIZE
  #define RULES_MAX_SIZE                   2048
#endif
#ifndef RULES_MAX_NESTING_LEVEL
  #define RULES_MAX_NESTING_LEVEL             3
#endif
#ifndef RULESETS_MAX
  #define RULESETS_MAX                        4
#endif
#ifndef RULES_BUFFER_SIZE
  #define RULES_BUFFER_SIZE                  64
#endif

#ifndef RULES_IF_MAX_NESTING_LEVEL
  #define RULES_IF_MAX_NESTING_LEVEL          4
#endif


// ***********************************************************************
// * Extended SecuritySettings
// ***********************************************************************
#ifndef EXT_SECURITY_MAX_USER_LENGTH
  #define EXT_SECURITY_MAX_USER_LENGTH        128
#endif
#ifndef EXT_SECURITY_MAX_PASS_LENGTH
  #define EXT_SECURITY_MAX_PASS_LENGTH        128
#endif

// ***********************************************************************
// * Other operational limits
// ***********************************************************************

#ifndef MAX_FLASHWRITES_PER_DAY
  #define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window
#endif
#ifndef UDP_PACKETSIZE_MAX
  #define UDP_PACKETSIZE_MAX               256 // Currently only needed for C013_Receive
#endif
#ifndef TIMER_GRATUITOUS_ARP_MAX
  #define TIMER_GRATUITOUS_ARP_MAX           5000
#endif

#define DOMOTICZ_MAX_IDX            999999999 // Looks like it is an unsigned int, so could be up to 4 bln.


#endif // CUSTOMBUILD_ESPEASY_LIMITS_H
