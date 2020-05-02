#include "CompiletimeDefines.h"

#ifndef BUILD_BINARY_FILENAME
// FIXME TD-er: Commented out to check if all PIO environments include the pre:tools/pio/generate-compiletime-defines.py
//#define BUILD_BINARY_FILENAME "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames"
#endif 

String get_build_filename() {
  return F(BUILD_BINARY_FILENAME);
}

String get_build_time() {
  return __TIME__;
}

String get_build_date() {
  return __DATE__;
}

bool official_build() {
  #ifdef CONTINUOUS_INTEGRATION
  return true;
  #else
  return false;
  #endif
}