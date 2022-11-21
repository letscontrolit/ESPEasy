#include "../CustomBuild/CompiletimeDefines.h"

#include "../CustomBuild/ESPEasy_buildinfo.h"

// This file will be "patched" at compiletime by
// tools/pio/generate-compiletime-defines.py
// Therefore this one may not include ESPEasy_common.h
//
// This Python script will define the following defines:

// # define SET_BUILD_BINARY_FILENAME "firmware.bin"
// # define SET_BUILD_PLATFORM "unknown"
// # define SET_BUILD_GIT_HEAD ""

// End of defines being patched by the Python build script.

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*

// Need to add quotes around defines as the PIO build tools make it hard to include the string quotes.
#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


uint16_t get_build_nr() {
  #ifdef SET_BUILD_VERSION
  return SET_BUILD_VERSION;
  #else
  #pragma message ( "Build is not based on current date" )
  // Last used version for BUILD was 20116, 
  // so make sure we can recognize a build not made using the compile time defines.
  return 20117; 
  #endif
}

const __FlashStringHelper* get_binary_filename() {
 #ifndef SET_BUILD_BINARY_FILENAME
  return F("firmware.bin");
 #else // ifndef SET_BUILD_BINARY_FILENAME
  return F(SET_BUILD_BINARY_FILENAME);
 #endif // ifndef SET_BUILD_BINARY_FILENAME
}

const __FlashStringHelper* get_build_time() {
  return F(__TIME__);
}

const __FlashStringHelper* get_build_date() {
  return F(__DATE__);
}

uint32_t get_build_unixtime() {
  #ifdef SET_BUILD_UNIXTIME
  return SET_BUILD_UNIXTIME;
  #else
  // Return some Unix time which we know is in the (somewhat recent) past
  return 1664582400; // Sat Oct 01 2022 00:00:00 GMT+0000
  #endif
}

const __FlashStringHelper * get_build_date_RFC1123() {
#ifdef SET_BUILD_TIME_RFC1123
  return F(SET_BUILD_TIME_RFC1123);
#else
  return F("-1");
#endif
}

const __FlashStringHelper* get_build_origin() {
  #if defined(CONTINUOUS_INTEGRATION)
  return F("GitHub Actions");
  #elif defined(VAGRANT_BUILD)
  return F("Vagrant");
  #else // if defined(CONTINUOUS_INTEGRATION)
  return F("Self built");
  #endif // if defined(CONTINUOUS_INTEGRATION)
}

const __FlashStringHelper* get_build_platform() {
 #ifndef SET_BUILD_PLATFORM
  return F("");
  #else // ifndef SET_BUILD_PLATFORM
  return F(SET_BUILD_PLATFORM);
 #endif // ifndef SET_BUILD_PLATFORM
}

const __FlashStringHelper* get_git_head() {
 #ifndef SET_BUILD_GIT_HEAD
  return F("");
 #else // ifndef SET_BUILD_GIT_HEAD
  return F(SET_BUILD_GIT_HEAD);
 #endif // ifndef SET_BUILD_GIT_HEAD
}

const __FlashStringHelper * get_board_name() {
  #ifdef SET_BOARD_NAME
  return F(SET_BOARD_NAME);
  #elif defined(ARDUINO_BOARD)
  return F(ARDUINO_BOARD);
  #else
  return F("");
  #endif
}

const __FlashStringHelper * get_CDN_url_prefix() {
  #ifdef CUSTOM_BUILD_CDN_URL
    return F(CUSTOM_BUILD_CDN_URL);
  #elif defined(SET_BUILD_CDN_URL)
    return F(SET_BUILD_CDN_URL);
  #else
    // Some fallback tag
    // FIXME TD-er: Not sure which is better, serving the latest (which will have caching issues) or a tag which will become outdated
    return F("https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy@mega-20220809/static/");
    //return F("https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy/static/");
  #endif
}