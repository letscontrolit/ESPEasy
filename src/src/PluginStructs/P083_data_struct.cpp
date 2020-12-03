#include "../PluginStructs/P083_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <Adafruit_SGP30.h>


#ifdef USES_P083

P083_data_struct::P083_data_struct() {
  initialized = sgp.begin();
  init_time   = millis();
}

#endif // ifdef USES_P083
