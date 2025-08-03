#ifndef DATATYPES_NODETYPEID_H
#define DATATYPES_NODETYPEID_H

#include "../../ESPEasy_common.h"

// These defines are used to identify other ESPEasy build platforms via ESPEasy p2p
// So don't ever change already assigned values as there might be older builds which rely on these.
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
#define NODE_TYPE_ID_ESP_EASY32C61_STD     40
#define NODE_TYPE_ID_ESP_EASY32C5_STD      41
#define NODE_TYPE_ID_ESP_EASY32P4_STD      42
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81


#if defined(ESP8266)
  # define NODE_TYPE_ID      NODE_TYPE_ID_ESP_EASYM_STD
#endif


#if defined(ESP32)
  # ifdef ESP32S2
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32S2_STD
  # elif defined(ESP32S3)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32S3_STD
  # elif defined(ESP32C2)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32C2_STD
  # elif defined(ESP32C3)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32C3_STD
  # elif defined(ESP32C5)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32C5_STD
  # elif defined(ESP32C6)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32C6_STD
  # elif defined(ESP32C61)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32C61_STD
  # elif defined(ESP32H2)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32H2_STD
  # elif defined(ESP32P4)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32P4_STD
  # elif defined(ESP32_CLASSIC)
    #  define NODE_TYPE_ID            NODE_TYPE_ID_ESP_EASY32_STD
  # else 

static_assert(false, "Implement processor architecture");

  # endif
#endif


const __FlashStringHelper* toNodeTypeDisplayString(uint8_t nodeType);

#endif // ifndef DATATYPES_NODETYPEID_H
