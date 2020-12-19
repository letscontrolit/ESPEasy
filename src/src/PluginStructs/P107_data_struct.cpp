#include "../PluginStructs/P107_data_struct.h"

#ifdef USES_P107

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <Adafruit_SI1145.h>

bool P107_data_struct::begin()
{
  return uv.begin();
}

#endif // ifdef USES_P107
