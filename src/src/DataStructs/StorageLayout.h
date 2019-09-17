#ifndef DATASTRUCTS_STORAGE_LAYOUT_H
#define DATASTRUCTS_STORAGE_LAYOUT_H


/*
These parameters determine the layout of the settings.
To see a graphic representation of the active settings in ESPEasy:
- Run the command:  meminfodetail
- Go to sysinfo page and scroll to end of page

These values can be overriden, either here, or via the Custom.h
The consequence is that the settings used with alternatives for these offsets will not be supported by other builds.

Some of these values are related to values set in ESPEasyLimits.h
*/

#ifndef DAT_TASKS_DISTANCE
#define DAT_TASKS_DISTANCE               2048  // DAT_TASKS_SIZE + DAT_TASKS_CUSTOM_SIZE
#endif
#ifndef DAT_TASKS_SIZE
#define DAT_TASKS_SIZE                   1024
#endif
#ifndef DAT_TASKS_CUSTOM_OFFSET
#define DAT_TASKS_CUSTOM_OFFSET          1024  // Equal to DAT_TASKS_SIZE
#endif
#ifndef DAT_TASKS_CUSTOM_SIZE
#define DAT_TASKS_CUSTOM_SIZE            1024
#endif
#ifndef DAT_CUSTOM_CONTROLLER_SIZE
#define DAT_CUSTOM_CONTROLLER_SIZE       1024
#endif
#ifndef DAT_CONTROLLER_SIZE
#define DAT_CONTROLLER_SIZE              1024
#endif
#ifndef DAT_NOTIFICATION_SIZE
#define DAT_NOTIFICATION_SIZE            1024
#endif
#ifndef DAT_BASIC_SETTINGS_SIZE
#define DAT_BASIC_SETTINGS_SIZE          4096
#endif

#if defined(ESP8266)
  #ifndef DAT_OFFSET_TASKS
  #define DAT_OFFSET_TASKS                 4096  // each task = 2k, (1024 basic + 1024 bytes custom), 12 max
  #endif
  #ifndef DAT_OFFSET_CONTROLLER
  #define DAT_OFFSET_CONTROLLER           28672  // each controller = 1k, 4 max
  #endif
  #ifndef DAT_OFFSET_CUSTOM_CONTROLLER
  #define DAT_OFFSET_CUSTOM_CONTROLLER    32768  // each custom controller config = 1k, 4 max.
  #endif
  #ifndef CONFIG_FILE_SIZE
  #define CONFIG_FILE_SIZE                65536
  #endif
#endif
#if defined(ESP32)
  #ifndef DAT_OFFSET_TASKS
  #define DAT_OFFSET_TASKS                 8192  // each controller = 1k, 4 max
  #endif
  #ifndef DAT_OFFSET_CONTROLLER
  #define DAT_OFFSET_CONTROLLER           12288  // each custom controller config = 1k, 4 max.
  #endif
  #ifndef DAT_OFFSET_CUSTOM_CONTROLLER
  #define DAT_OFFSET_CUSTOM_CONTROLLER    32768  // each task = 2k, (1024 basic + 1024 bytes custom), 32 max
  #endif
  #ifndef CONFIG_FILE_SIZE
  #define CONFIG_FILE_SIZE               131072
  #endif
#endif


#endif // DATASTRUCTS_STORAGE_LAYOUT_H