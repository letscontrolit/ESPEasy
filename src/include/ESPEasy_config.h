#ifndef INCLUDE_ESPEASY_CONFIG_H
#define INCLUDE_ESPEASY_CONFIG_H

#ifdef __cplusplus

//#include <Arduino.h>

//#include <stddef.h>

//#include <stdint.h>
//#include <string.h>


#ifdef __GCC__
#pragma GCC system_header
#endif


/******************************************************************************\
 * Detect core versions *******************************************************
\******************************************************************************/

#ifndef ESP32
  #if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
    #ifndef CORE_2_4_X
      #define CORE_2_4_X
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
    #ifndef CORE_PRE_2_4_2
      #define CORE_PRE_2_4_2
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #ifndef CORE_PRE_2_5_0
      #define CORE_PRE_2_5_0
    #endif
  #else
    #ifndef CORE_POST_2_5_0
      #define CORE_POST_2_5_0
    #endif
  #endif


  #ifdef FORCE_PRE_2_5_0
    #ifdef CORE_POST_2_5_0
      #undef CORE_POST_2_5_0
    #endif
  #endif

/*
  #ifndef CORE_POST_2_5_0
 #define STR_HELPER(x) #x
 #define STR(x) STR_HELPER(x)
  #endif
*/

#endif


#if defined(ESP8266)
  #include <c_types.h>

  #ifndef CORE_POST_3_0_0
    #ifndef IRAM_ATTR
      #define IRAM_ATTR ICACHE_RAM_ATTR
    #endif
  #endif
#endif

#if defined(ESP32)
  #include <esp8266-compat.h>
  #if ESP_IDF_VERSION_MAJOR < 3
    #ifndef ICACHE_RAM_ATTR
      #define ICACHE_RAM_ATTR IRAM_ATTR
    #endif
  #endif
#endif

#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0

#ifdef ESP32
  // Special macros to disable interrupts from within an ISR function.
  //
  // See:
  // Re-added interrupt code again for ESP32:
  // https://github.com/espressif/arduino-esp32/commit/55d608e322443b7ac080b0ab62d5a93f26d2212f
  // However this code cannot be called from an interrupt service routine.
  // See comments at portDISABLE_INTERRUPTS
  #  define ISR_noInterrupts()                           \
  portMUX_TYPE updateMux = portMUX_INITIALIZER_UNLOCKED; \
  portTRY_ENTER_CRITICAL_ISR(&updateMux, 1000);

  #  define ISR_interrupts() portEXIT_CRITICAL(&updateMux);
# endif // ifdef ESP32
# ifdef ESP8266
  #  define ISR_noInterrupts() noInterrupts();
  #  define ISR_interrupts() interrupts();
# endif // ifdef ESP8266


// User configuration
// Include Custom.h before ESPEasyDefaults.h.
# ifdef USE_CUSTOM_H

// make the compiler show a warning to confirm that this file is included
// #warning "**** Using Settings from Custom.h File ***"
  #  include "../Custom.h"
# endif // ifdef USE_CUSTOM_H

// Check if any deprecated '#define <variable>' (Custom.h) or '-D<variable>' (pre_custom_esp82xx.py/pre_custom_esp32.py) are used

# include "../src/CustomBuild/check_defines_custom.h" // Check for replaced #define variables, see
                                                      // https://github.com/letscontrolit/ESPEasy/pull/4153

// Include custom first, then build info. (one may want to set BUILD_GIT for example)
# include "../src/CustomBuild/ESPEasy_buildinfo.h"
# include "../src/CustomBuild/ESPEasyLimits.h"
# include "../src/CustomBuild/define_plugin_sets.h"


#endif // ifdef __cplusplus

#endif // ifndef INCLUDE_ESPEASY_CONFIG_H
