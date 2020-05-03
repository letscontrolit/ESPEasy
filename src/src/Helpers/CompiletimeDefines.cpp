#include "CompiletimeDefines.h"

#ifndef SET_BUILD_BINARY_FILENAME
#define SET_BUILD_BINARY_FILENAME "firmware.bin"
#endif // ifndef SET_BUILD_BINARY_FILENAME

#ifndef SET_BUILD_PLATFORM
# define SET_BUILD_PLATFORM "unknown"
#endif // ifndef SET_BUILD_PLATFORM

#ifndef SET_BUILD_GIT_HEAD
#define SET_BUILD_GIT_HEAD ""
#endif

String get_binary_filename() {
  return F(SET_BUILD_BINARY_FILENAME);
}

String get_build_time() {
  return F(__TIME__);
}

String get_build_date() {
  return F(__DATE__);
}

bool official_build() {
  #ifdef CONTINUOUS_INTEGRATION
  return true;
  #else // ifdef CONTINUOUS_INTEGRATION
  return false;
  #endif // ifdef CONTINUOUS_INTEGRATION
}

String get_build_platform() {
  return F(SET_BUILD_PLATFORM);
}

String get_git_head() {
  return F(SET_BUILD_GIT_HEAD);
}