#include "CompiletimeDefines.h"

#ifndef SET_BUILD_BINARY_FILENAME
# define SET_BUILD_BINARY_FILENAME "firmware.bin"
#endif // ifndef SET_BUILD_BINARY_FILENAME

#ifndef SET_BUILD_PLATFORM
# define SET_BUILD_PLATFORM "unknown"
#endif // ifndef SET_BUILD_PLATFORM

#ifndef SET_BUILD_GIT_HEAD
# define SET_BUILD_GIT_HEAD ""
#endif // ifndef SET_BUILD_GIT_HEAD

String get_binary_filename() {
  return F(SET_BUILD_BINARY_FILENAME);
}

String get_build_time() {
  return F(__TIME__);
}

String get_build_date() {
  return F(__DATE__);
}

String get_build_origin() {
  #if defined(CONTINUOUS_INTEGRATION)
  return F("Travis");
  #elif defined(VAGRANT_BUILD)
  return F("Vagrant");
  #else 
  return F("Self built");
  #endif
}

String get_build_platform() {
  return F(SET_BUILD_PLATFORM);
}

String get_git_head() {
  return F(SET_BUILD_GIT_HEAD);
}
