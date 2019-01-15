#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

// Defines to make plugins more readable.

#ifndef PCONFIG
  #define PCONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][(n)])
#endif
#ifndef PCONFIG_FLOAT
  #define PCONFIG_FLOAT(n) (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][(n)])
#endif
#ifndef PCONFIG_LONG
  #define PCONFIG_LONG(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][(n)])
#endif
#ifndef PIN
  // Please note the 'offset' of N compared to normal pin numbering.
  #define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif
#ifndef CONFIG_PIN1
  #define CONFIG_PIN1 (Settings.TaskDevicePin1[event->TaskIndex])
#endif
#ifndef CONFIG_PIN2
  #define CONFIG_PIN2 (Settings.TaskDevicePin2[event->TaskIndex])
#endif
#ifndef CONFIG_PIN3
  #define CONFIG_PIN3 (Settings.TaskDevicePin3[event->TaskIndex])
#endif
#ifndef CONFIG_PORT
  #define CONFIG_PORT (Settings.TaskDevicePort[event->TaskIndex])
#endif


#endif // PLUGIN_HELPER_H
