#ifndef DATASTRUCTS_ESPEASY_LIMITS_H
#define DATASTRUCTS_ESPEASY_LIMITS_H

// ***********************************************************************
// * These limits have direct impact on the settings files
// * Do not change them!
// * Else settings files will no longer be compatible with official builds
// ***********************************************************************

#if defined(ESP8266)
  #ifndef TASKS_MAX
    #define TASKS_MAX                          12 // max 12!
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
  #define MAX_GPIO                           39
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



// ***********************************************************************
// * The next limits affect memory usage
// ***********************************************************************

#ifndef DEVICES_MAX
  #if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
    #define DEVICES_MAX                      95
  #else
    #ifdef ESP32
      #define DEVICES_MAX                      85
    #else
      #define DEVICES_MAX                      60
    #endif
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

#ifndef UNIT_MAX
  #define UNIT_MAX                          254 // unit 255 = broadcast
#endif
#ifndef CUSTOM_VARS_MAX
  #define CUSTOM_VARS_MAX                    16
#endif



// ***********************************************************************
// * Limits regarding Rules
// ***********************************************************************

#ifndef RULES_TIMER_MAX
  #define RULES_TIMER_MAX                     8
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


#ifndef INPUT_COMMAND_SIZE
  #define INPUT_COMMAND_SIZE                240 // Affects maximum command length in rules and other commands
#endif
// FIXME TD-er: INPUT_COMMAND_SIZE is also used in commands where simply a check for valid parameter is needed
// and some may need less memory. (which is stack allocated)


// ***********************************************************************
// * Other operational limits
// ***********************************************************************

#ifndef MAX_FLASHWRITES_PER_DAY
  #define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window
#endif
#ifndef UDP_PACKETSIZE_MAX
  #define UDP_PACKETSIZE_MAX               2048
#endif
#ifndef TIMER_GRATUITOUS_ARP_MAX
  #define TIMER_GRATUITOUS_ARP_MAX           5000
#endif

#ifndef UNIT_NUMBER_MAX
  #define UNIT_NUMBER_MAX                  9999  // Stored in Settings.Unit
#endif
#ifndef DOMOTICZ_MAX_IDX
  #define DOMOTICZ_MAX_IDX            999999999  // Looks like it is an unsigned int, so could be up to 4 bln.
#endif





#endif // DATASTRUCTS_ESPEASY_LIMITS_H
