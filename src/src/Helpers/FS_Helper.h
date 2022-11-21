#ifndef HELPERS_SPIFFS_HELPER_H
#define HELPERS_SPIFFS_HELPER_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

// Macro used to make file system operations a bit more readable.
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }

#ifdef USE_LITTLEFS
  #ifdef ESP32
    #if ESP_IDF_VERSION_MAJOR >= 4
      #include <LittleFS.h>
      #define ESPEASY_FS LittleFS
    #else
      #include <LITTLEFS.h>
      #define ESPEASY_FS LITTLEFS
    #endif
  #else
    #include <LittleFS.h>
    #define ESPEASY_FS LittleFS
  #endif
#else 
  #ifdef ESP32
    #include <SPIFFS.h>
  #endif
  #define ESPEASY_FS SPIFFS
#endif


#include <FS.h>
#if FEATURE_SD
#include <SD.h>
#else
using namespace fs;
#endif


#if defined(ESP8266)
  extern "C" {
    #include <spi_flash.h>
  }
  #ifdef CORE_POST_2_6_0
    extern "C" uint32_t _FS_start;
    extern "C" uint32_t _FS_end;
    extern "C" uint32_t _FS_page;
    extern "C" uint32_t _FS_block;
  #else
    extern "C" uint32_t _SPIFFS_start;
    extern "C" uint32_t _SPIFFS_end;
    extern "C" uint32_t _SPIFFS_page;
    extern "C" uint32_t _SPIFFS_block;
  #endif
#endif

#endif