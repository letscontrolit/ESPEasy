#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

#include "ESPEasy_common.h"

#include "src/CustomBuild/ESPEasyLimits.h"

#include "src/DataStructs/DeviceStruct.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/PinMode.h"
#include "src/DataStructs/PluginTaskData_base.h"

#include "src/DataTypes/ESPEasy_plugin_functions.h"

#include "src/ESPEasyCore/Controller.h"
#include "src/ESPEasyCore/ESPEasy_Log.h"
#include "src/ESPEasyCore/Serial.h"

#include "src/Globals/Cache.h"
#include "src/Globals/Device.h"
#include "src/Globals/ESPEasy_Scheduler.h"
#include "src/Globals/ESPEasy_time.h"
#include "src/Globals/EventQueue.h"
#include "src/Globals/ExtraTaskSettings.h"
#include "src/Globals/GlobalMapPortStatus.h"
#include "src/Globals/I2Cdev.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/RuntimeData.h"
#include "src/Globals/Settings.h"
#include "src/Globals/Services.h"

#include "src/Helpers/_Plugin_init.h"
#include "src/Helpers/ESPEasy_math.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/ESPEasy_time_calc.h"
#include "src/Helpers/I2C_access.h"
#include "src/Helpers/Hardware.h"
#include "src/Helpers/Hardware_GPIO.h"
#include "src/Helpers/Hardware_PWM.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/Numerical.h"
#include "src/Helpers/PortStatus.h"
#include "src/Helpers/PrintToString.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringGenerator_GPIO.h"
#include "src/Helpers/StringGenerator_Plugin.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/TaskValuesWriterHelper.h"
#include "src/Helpers/_Plugin_SensorTypeHelper.h"
#include "src/Helpers/_Plugin_Helper_serial.h"

#if FEATURE_MQTT_DISCOVER
# include "src/Helpers/_CPlugin_Helper_mqtt.h"
#endif // if FEATURE_MQTT_DISCOVER

#if FEATURE_PLUGIN_STATS
# include "src/PluginStructs/_StatsOnly_data_struct.h"
#endif

#include "src/WebServer/Chart_JS.h"
#include "src/WebServer/HTML_wrappers.h"
#include "src/WebServer/Markup.h"
#include "src/WebServer/Markup_Forms.h"
#include "src/WebServer/ESPEasy_WebServer.h"

#if DEBUG_PCONFIG_RANGE_CHECK
# if DEBUG_PCONFIG_RANGE_CHECK > 2
#  define DEBUG_PCONFIG_RANGE_CHECK_args_decl   const struct EventStruct *event, uint8_t n, uint16_t linenr, \
        const __FlashStringHelper *filename
# elif DEBUG_PCONFIG_RANGE_CHECK > 1
#  define DEBUG_PCONFIG_RANGE_CHECK_args_decl   const struct EventStruct *event, uint8_t n, uint16_t linenr
# else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#  define DEBUG_PCONFIG_RANGE_CHECK_args_decl   const struct EventStruct *event, uint8_t n
# endif // if DEBUG_PCONFIG_RANGE_CHECK > 2

int16_t & do_PCONFIG(DEBUG_PCONFIG_RANGE_CHECK_args_decl);
float   & do_PCONFIG_FLOAT(DEBUG_PCONFIG_RANGE_CHECK_args_decl);
int32_t & do_PCONFIG_LONG(DEBUG_PCONFIG_RANGE_CHECK_args_decl);
uint32_t& do_PCONFIG_ULONG(DEBUG_PCONFIG_RANGE_CHECK_args_decl);
int8_t  & do_PIN(DEBUG_PCONFIG_RANGE_CHECK_args_decl);
#endif // if DEBUG_PCONFIG_RANGE_CHECK


// Defines to make plugins more readable.
#ifndef PCONFIG
# if DEBUG_PCONFIG_RANGE_CHECK
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG(n) do_PCONFIG(event, n, __LINE__, F(__FILE__))
#  elif DEBUG_PCONFIG_RANGE_CHECK > 1
#   define PCONFIG(n) do_PCONFIG(event, n, __LINE__)
#  else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG(n) do_PCONFIG(event, n)
#  endif // if DEBUG_PCONFIG_RANGE_CHECK > 2
# else // if DEBUG_PCONFIG_RANGE_CHECK
#  define PCONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][(n)])
# endif // if DEBUG_PCONFIG_RANGE_CHECK
#endif // ifndef PCONFIG
#ifndef PCONFIG_FLOAT
# if DEBUG_PCONFIG_RANGE_CHECK
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_FLOAT(n) do_PCONFIG_FLOAT(event, n, __LINE__, F(__FILE__))
#  elif DEBUG_PCONFIG_RANGE_CHECK > 1
#   define PCONFIG_FLOAT(n) do_PCONFIG_FLOAT(event, n, __LINE__)
#  else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_FLOAT(n) do_PCONFIG_FLOAT(event, n)
#  endif // if DEBUG_PCONFIG_RANGE_CHECK > 2
# else // if DEBUG_PCONFIG_RANGE_CHECK
#  define PCONFIG_FLOAT(n) (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][(n)])
# endif // if DEBUG_PCONFIG_RANGE_CHECK
#endif // ifndef PCONFIG_FLOAT
#ifndef PCONFIG_LONG
# if DEBUG_PCONFIG_RANGE_CHECK
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_LONG(n) do_PCONFIG_LONG(event, n, __LINE__, F(__FILE__))
#  elif DEBUG_PCONFIG_RANGE_CHECK > 1
#   define PCONFIG_LONG(n) do_PCONFIG_LONG(event, n, __LINE__)
#  else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_LONG(n) do_PCONFIG_LONG(event, n)
#  endif // if DEBUG_PCONFIG_RANGE_CHECK > 2
# else // if DEBUG_PCONFIG_RANGE_CHECK
#  define PCONFIG_LONG(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][(n)])
# endif // if DEBUG_PCONFIG_RANGE_CHECK
#endif // ifndef PCONFIG_LONG
#ifndef PCONFIG_ULONG
# if DEBUG_PCONFIG_RANGE_CHECK
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_ULONG(n) do_PCONFIG_ULONG(event, n, __LINE__, F(__FILE__))
#  elif DEBUG_PCONFIG_RANGE_CHECK > 1
#   define PCONFIG_ULONG(n) do_PCONFIG_ULONG(event, n, __LINE__)
#  else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PCONFIG_ULONG(n) do_PCONFIG_ULONG(event, n)
#  endif // if DEBUG_PCONFIG_RANGE_CHECK > 2
# else // if DEBUG_PCONFIG_RANGE_CHECK
#  define PCONFIG_ULONG(n) (Settings.TaskDevicePluginConfigULong[event->TaskIndex][(n)])
# endif // if DEBUG_PCONFIG_RANGE_CHECK
#endif // ifndef PCONFIG_ULONG
#ifndef PIN

// Please note the 'offset' of N compared to normal pin numbering.
# if DEBUG_PCONFIG_RANGE_CHECK
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PIN(n) do_PIN(event, n, __LINE__, F(__FILE__))
#  elif DEBUG_PCONFIG_RANGE_CHECK > 1
#   define PIN(n) do_PIN(event, n, __LINE__)
#  else // if DEBUG_PCONFIG_RANGE_CHECK > 2
#   define PIN(n) do_PIN(event, n)
#  endif // if DEBUG_PCONFIG_RANGE_CHECK > 2
# else // if DEBUG_PCONFIG_RANGE_CHECK
#  define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
# endif // if DEBUG_PCONFIG_RANGE_CHECK
#endif // ifndef PIN
#ifndef CONFIG_PIN1
# define CONFIG_PIN1 (Settings.TaskDevicePin1[event->TaskIndex])
#endif // ifndef CONFIG_PIN1
#ifndef CONFIG_PIN2
# define CONFIG_PIN2 (Settings.TaskDevicePin2[event->TaskIndex])
#endif // ifndef CONFIG_PIN2
#ifndef CONFIG_PIN3
# define CONFIG_PIN3 (Settings.TaskDevicePin3[event->TaskIndex])
#endif // ifndef CONFIG_PIN3
#ifndef CONFIG_PORT
# define CONFIG_PORT (Settings.TaskDevicePort[event->TaskIndex])
#endif // ifndef CONFIG_PORT


extern PluginTaskData_base *Plugin_task_data[TASKS_MAX];

// Try to allocate in PSRAM or 2nd heap if possible
#define special_initPluginTaskData(I, T) void *ptr = special_calloc(1, sizeof(T)); \
        if (ptr) { initPluginTaskData(I, new (ptr) T()); }

String PCONFIG_LABEL(int n);

// ==============================================
// Data used by instances of plugins.
// =============================================

void                 resetPluginTaskData();

void                 clearPluginTaskData(taskIndex_t taskIndex);

bool                 initPluginTaskData(taskIndex_t          taskIndex,
                                        PluginTaskData_base *data);

PluginTaskData_base* getPluginTaskData(taskIndex_t taskIndex);
PluginTaskData_base* getPluginTaskDataBaseClassOnly(taskIndex_t taskIndex);

bool                 pluginTaskData_initialized(taskIndex_t taskIndex);

String               getPluginCustomArgName(int varNr);
String               getPluginCustomArgName(const __FlashStringHelper *label,
                                            int                        varNr);

int                  getFormItemIntCustomArgName(int varNr);


// Check if given parameter nr matches with given taskIndex.
// paramNr == 0 -> command, paramNr == 1 -> 1st parameter
// When there is no parameter at given parameter position, this function will return true. (as it is an optional parameter)
// When given taskIndex is invalid, return value is false.
// Return if parameter at given paramNr matches given taskIndex.
bool pluginOptionalTaskIndexArgumentMatch(taskIndex_t   taskIndex,
                                          const String& string,
                                          uint8_t       paramNr);

bool pluginWebformShowGPIOdescription(taskIndex_t                taskIndex,
                                      const __FlashStringHelper *newline,
                                      String                   & description);

int getValueCountForTask(taskIndex_t taskIndex);

// Check if the DeviceVType is set and update if it isn't.
// Return pconfig_index
int checkDeviceVTypeForTask(struct EventStruct *event);

#endif // PLUGIN_HELPER_H
