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
  #include "esp32x_fixes.h"
  #include <esp8266-compat.h>
  #if ESP_IDF_VERSION_MAJOR < 3
    #ifndef ICACHE_RAM_ATTR
      #define ICACHE_RAM_ATTR IRAM_ATTR
    #endif
  #endif
#endif

#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0

#define NR_ELEMENTS(ARR)   (sizeof (ARR) / sizeof *(ARR))
//#define NR_ELEMENTS(ARR) sizeof(ARR) / sizeof(ARR[0])


constexpr unsigned FLOOR_LOG2(unsigned x)
{
  return x == 1 ? 0 : (1 + FLOOR_LOG2(x >> 1));
}

constexpr unsigned CEIL_LOG2(unsigned x)
{
  return x == 1 ? 0 : (FLOOR_LOG2(x - 1) + 1);
}

// Compute at compile time the number of bits required to store N states
# define NR_BITS(NR_STATES) CEIL_LOG2(NR_STATES)

// Compute a mask given number of bits
# define MASK_BITS(x) ((1 << (x)) - 1)


#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s


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


#if ESP_IDF_VERSION_MAJOR >= 5
  #include <atomic>

  #define ESPEASY_VOLATILE(T)  std::atomic<T>
#else
  #define ESPEASY_VOLATILE(T)  volatile T
#endif
  


# endif // ifdef ESP32
# ifdef ESP8266
  #  define ISR_noInterrupts() noInterrupts();
  #  define ISR_interrupts() interrupts();

  #define ESPEASY_VOLATILE(T)  volatile T
# endif // ifdef ESP8266


# ifdef ESP8266

  // (ESP8266) FsP: FlashstringHelper to String-Pointer
  #  define FsP(F) String(F).c_str()
# endif // ifdef ESP8266

# ifdef ESP32
  #  if defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)

    // (ESP32) FsP: FlashstringHelper to String-Pointer
    #   define FsP
  #  endif // if defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)
  #  if defined(ESP32_CLASSIC) || defined(ESP32S2) || defined(ESP32S3)

    // (ESP32) FsP: FlashstringHelper to String-Pointer
    #   define FsP
  #  endif // if defined(ESP32_CLASSIC) || defined(ESP32S2) || defined(ESP32S3)
# endif // ifdef ESP32

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
