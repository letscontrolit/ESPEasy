#ifndef HELPERS_SPIFFS_HELPER_H
#define HELPERS_SPIFFS_HELPER_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

// Macro used to make file system operations a bit more readable.
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }

#define FS_NO_GLOBALS
#if defined(ESP8266)
  extern "C" {
    #include "spi_flash.h"
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

#if defined(ESP32)
  #ifdef USE_LITTLEFS
    #include "LittleFS.h"
  #else
    #include "SPIFFS.h"
  #endif
#endif

#endif