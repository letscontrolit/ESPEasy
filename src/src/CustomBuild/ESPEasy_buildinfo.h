#ifndef CUSTOMBUILD_ESPEASY_BUILD_INFO_H
#define CUSTOMBUILD_ESPEASY_BUILD_INFO_H


// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
//   The following should also not be changed by defines in Custom.h:
//   - ESP_PROJECT_PID
//   - VERSION
//   - BUILD
// ********************************************************************************
#define ESP_PROJECT_PID           2016110801L

#if defined(ESP8266)
  # define VERSION                             2 // config file version (not ESPEasy version). increase if you make incompatible changes to
                                                 // config system.
#endif // if defined(ESP8266)
#if defined(ESP32)
  # define VERSION                             3 // Change in config.dat mapping needs a full reset
#endif // if defined(ESP32)


// Deprecated define.
// Use get_build_nr() from CustomBuild/CompiletimeDefines.h
//#define BUILD                           20116    // git version e.g. "20103" can be read as "2.1.03" (stored in int16_t)


#ifndef BUILD_NOTES
#if defined(ESP8266)
  # define BUILD_NOTES                 " - Mega"
#endif // if defined(ESP8266)
#if defined(ESP32)
  #if defined(ESP32S2)
    # define BUILD_NOTES                 " - Mega32-s2"
  #elif defined(ESP32C2)
    # define BUILD_NOTES                 " - Mega32-c3"
  #else
    # define BUILD_NOTES                 " - Mega32"
  #endif
#endif // if defined(ESP32)
#endif

#ifndef BUILD_GIT
# define BUILD_GIT "(custom)"
#endif // ifndef BUILD_GIT

// Development of ESPEasy-NOW layer has been paid for by a customer who agreed to make it Open Source.
// Therefore they use a different name in their builds.
#ifndef ESPEASY_NOW_NAME
# define ESPEASY_NOW_NAME "ESPEasy-NOW"
#endif

#endif // CUSTOMBUILD_ESPEASY_BUILD_INFO_H
