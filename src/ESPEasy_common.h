#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

#include <stddef.h>
namespace std
{
  using ::ptrdiff_t;
  using ::size_t;
}

#include <stdint.h>
#include <Arduino.h>

#ifdef USE_CUSTOM_H
#include "Custom.h"
#endif


#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0


String getUnknownString();

/*********************************************************************************************\
   Bitwise operators
  \*********************************************************************************************/
bool getBitFromUL(uint32_t number, byte bitnr);
void setBitToUL(uint32_t& number, byte bitnr, bool value);


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
#endif // ESP32

#endif // ESPEASY_COMMON_H