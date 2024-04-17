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
  #elif defined(ESP32S3)
    # define BUILD_NOTES                 " - Mega32-s3"
  #elif defined(ESP32C6)
    # define BUILD_NOTES                 " - Mega32-c6"
  #elif defined(ESP32C3)
    # define BUILD_NOTES                 " - Mega32-c3"
  #elif defined(ESP32C2)
    # define BUILD_NOTES                 " - Mega32-c2"
  # elif defined(ESP32_CLASSIC)
    # define BUILD_NOTES                 " - Mega32"
  # else
    static_assert(false, "Implement processor architecture");
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

#ifdef ESP32

/*
 #if CONFIG_IDF_TARGET_ESP32C6 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32S2 ||  // support USB via USBCDC
     CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
 */
# if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3

// #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if ARDUINO_USB_MODE

// ESP32C3/S3 embedded USB using JTAG interface
#    define USES_HWCDC 1
#   else // No ARDUINO_USB_MODE
#    define USES_USBCDC 1
#   endif // if ARDUINO_USB_MODE
#  endif // ifdef USE_USB_CDC_CONSOLE
# endif // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif // ifdef ESP32

#ifdef USES_USBCDC
#ifdef USB_MANUFACTURER
#undef USB_MANUFACTURER
#endif
#define USB_MANUFACTURER "ESPEasy"
#endif


#endif // CUSTOMBUILD_ESPEASY_BUILD_INFO_H
