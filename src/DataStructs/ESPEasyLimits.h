#ifndef DATASTRUCTS_ESPEASY_LIMITS_H
#define DATASTRUCTS_ESPEASY_LIMITS_H

#if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
  #define DEVICES_MAX                      95
#else
  #ifdef ESP32
    #define DEVICES_MAX                      85
  #else
    #define DEVICES_MAX                      60
  #endif
#endif

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
#define PLUGIN_MAX                DEVICES_MAX
#define PLUGIN_CONFIGVAR_MAX                8
#define PLUGIN_CONFIGFLOATVAR_MAX           4
#define PLUGIN_CONFIGLONGVAR_MAX            4
#define PLUGIN_EXTRACONFIGVAR_MAX          16
#define CPLUGIN_MAX                        20
#define NPLUGIN_MAX                         4
#define UNIT_MAX                          254 // unit 255 = broadcast
#define RULES_TIMER_MAX                     8
//#define PINSTATE_TABLE_MAX                 32
#define RULES_MAX_SIZE                   2048
#define RULES_MAX_NESTING_LEVEL             3
#define RULESETS_MAX                        4
#define RULES_BUFFER_SIZE                  64
#define NAME_FORMULA_LENGTH_MAX            40
#define RULES_IF_MAX_NESTING_LEVEL          4
#define CUSTOM_VARS_MAX                    16

#define UDP_PACKETSIZE_MAX               2048



#endif // DATASTRUCTS_ESPEASY_LIMITS_H