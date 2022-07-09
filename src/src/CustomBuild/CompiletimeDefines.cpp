#include "../CustomBuild/CompiletimeDefines.h"

// This file will be "patched" at compiletime by
// tools/pio/generate-compiletime-defines.py
// Therefore this one may not include ESPEasy_common.h
//
// This Python script will define the following defines:

// # define SET_BUILD_BINARY_FILENAME "firmware.bin"
// # define SET_BUILD_PLATFORM "unknown"
// # define SET_BUILD_GIT_HEAD ""

// End of defines being patched by the Python build script.

// Need to add quotes around defines as the PIO build tools make it hard to include the string quotes.
#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) # s

const __FlashStringHelper* get_binary_filename() {
 #ifndef SET_BUILD_BINARY_FILENAME
  return F("firmware.bin");
 #else // ifndef SET_BUILD_BINARY_FILENAME
  return F(STRINGIFY(SET_BUILD_BINARY_FILENAME));
 #endif // ifndef SET_BUILD_BINARY_FILENAME
}

const __FlashStringHelper* get_build_time() {
  return F(__TIME__);
}

const __FlashStringHelper* get_build_date() {
  return F(__DATE__);
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
  return F(STRINGIFY(SET_BUILD_PLATFORM));
 #endif // ifndef SET_BUILD_PLATFORM
}

const __FlashStringHelper* get_git_head() {
 #ifndef SET_BUILD_GIT_HEAD
  return F("");
 #else // ifndef SET_BUILD_GIT_HEAD
  return F(STRINGIFY(SET_BUILD_GIT_HEAD));
 #endif // ifndef SET_BUILD_GIT_HEAD
}

const __FlashStringHelper * get_board_name() {
  #ifdef SET_BOARD_NAME
  return F(STRINGIFY(SET_BOARD_NAME));
  #elif defined(ARDUINO_BOARD)
  return F(ARDUINO_BOARD);
  #else
  return F("");
  #endif
}