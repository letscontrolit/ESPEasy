#ifndef DATATYPES_NODETYPEID_H
#define DATATYPES_NODETYPEID_H

#include "../../ESPEasy_common.h"

#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_RPI_EASY_STD           5 // https://github.com/enesbcs/rpieasy
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ESP_EASY32S2_STD      34
#define NODE_TYPE_ID_ESP_EASY32C3_STD      35
#define NODE_TYPE_ID_ESP_EASY32S3_STD      36
#define NODE_TYPE_ID_ESP_EASY32C2_STD      37
#define NODE_TYPE_ID_ESP_EASY32H2_STD      38
#define NODE_TYPE_ID_ESP_EASY32C6_STD      39
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81


#if defined(ESP8266)
  #define NODE_TYPE_ID      NODE_TYPE_ID_ESP_EASYM_STD
#endif


#if defined(ESP32)
  #ifdef ESP32S2
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32S2_STD
  #elif defined(ESP32S3)
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32S3_STD
  #elif defined(ESP32C3)
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32C3_STD
  # elif defined(ESP32_CLASSIC)
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32_STD
  # else

    static_assert(false, "Implement processor architecture");

  #endif
#endif



const __FlashStringHelper* toNodeTypeDisplayString(uint8_t nodeType);

#endif // ifndef DATATYPES_NODETYPEID_H