#include "CompiletimeDefines.h"

// This file will be "patched" at compiletime by 
// tools/pio/generate-compiletime-defines.py
// Therefore this one may not include ESPEasy_common.h
//
// This Python script will define the following defines:

#ifndef SET_BUILD_BINARY_FILENAME
# define SET_BUILD_BINARY_FILENAME "firmware.bin"
#endif // ifndef SET_BUILD_BINARY_FILENAME

#ifndef SET_BUILD_PLATFORM
# define SET_BUILD_PLATFORM "unknown"
#endif // ifndef SET_BUILD_PLATFORM

#ifndef SET_BUILD_GIT_HEAD
# define SET_BUILD_GIT_HEAD ""
#endif // ifndef SET_BUILD_GIT_HEAD


// End of defines being patched by the Python build script.

const __FlashStringHelper * get_binary_filename() {
 #if !defined(CORE_POST_2_5_0) && !defined(ESP32)
   return F("firmware.bin");
 #else
    return F(SET_BUILD_BINARY_FILENAME);
 #endif
}

const __FlashStringHelper * get_build_time() {
  return F(__TIME__);
}

const __FlashStringHelper * get_build_date() {
  return F(__DATE__);
}

const __FlashStringHelper * get_build_origin() {
  #if defined(CONTINUOUS_INTEGRATION)
  return F("GitHub Actions");
  #elif defined(VAGRANT_BUILD)
  return F("Vagrant");
  #else 
  return F("Self built");
  #endif
}

const __FlashStringHelper * get_build_platform() {
 #if !defined(CORE_POST_2_5_0) && !defined(ESP32)
    return F("");
  #else
    return F(SET_BUILD_PLATFORM);
 #endif
}

const __FlashStringHelper * get_git_head() {
 #if !defined(CORE_POST_2_5_0) && !defined(ESP32)
   return F("");
 #else
    return F(SET_BUILD_GIT_HEAD);
 #endif
}
