#ifndef DATASTRUCTS_ESPEASY_LIMITS_H
#define DATASTRUCTS_ESPEASY_LIMITS_H

// ***********************************************************************
// * These limits have direct impact on the settings files
// * Do not change them!
// * Else settings files will no longer be compatible with official builds
// ***********************************************************************

#if defined(ESP8266)
  #define TASKS_MAX                          12 // max 12!
  #define MAX_GPIO                           16
#endif
#if defined(ESP32)
  #define TASKS_MAX                          32
  #define MAX_GPIO                           39
#endif

#define CONTROLLER_MAX                      3 // max 4!
#define NOTIFICATION_MAX                    3 // max 4!
#define VARS_PER_TASK                       4
#define PLUGIN_CONFIGVAR_MAX                8
#define PLUGIN_CONFIGFLOATVAR_MAX           4
#define PLUGIN_CONFIGLONGVAR_MAX            4
#define PLUGIN_EXTRACONFIGVAR_MAX          16
#define NAME_FORMULA_LENGTH_MAX            40



// ***********************************************************************
// * The next limits affect memory usage
// ***********************************************************************

#if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
  #define DEVICES_MAX                      95
#else
  #ifdef ESP32
    #define DEVICES_MAX                      85
  #else
    #define DEVICES_MAX                      60
  #endif
#endif

#define PLUGIN_MAX                DEVICES_MAX
#define CPLUGIN_MAX                        20
#define NPLUGIN_MAX                         4

#define UNIT_MAX                          254 // unit 255 = broadcast
#define CUSTOM_VARS_MAX                    16



// ***********************************************************************
// * Limits regarding Rules
// ***********************************************************************

#define RULES_TIMER_MAX                     8
//#define PINSTATE_TABLE_MAX                 32
#define RULES_MAX_SIZE                   2048
#define RULES_MAX_NESTING_LEVEL             3
#define RULESETS_MAX                        4
#define RULES_BUFFER_SIZE                  64

#define RULES_IF_MAX_NESTING_LEVEL          4


#define INPUT_COMMAND_SIZE                240 // Affects maximum command length in rules and other commands
// FIXME TD-er: INPUT_COMMAND_SIZE is also used in commands where simply a check for valid parameter is needed
// and some may need less memory. (which is stack allocated)


// ***********************************************************************
// * Other operational limits
// ***********************************************************************

#define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window
#define UDP_PACKETSIZE_MAX               2048
#define TIMER_GRATUITOUS_ARP_MAX           5000

#define UNIT_NUMBER_MAX                  9999  // Stored in Settings.Unit
#define DOMOTICZ_MAX_IDX            999999999  // Looks like it is an unsigned int, so could be up to 4 bln.





#endif // DATASTRUCTS_ESPEASY_LIMITS_H