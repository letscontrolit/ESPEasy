#include "P083_data_struct.h"


#ifdef USES_P083

P083_data_struct::P083_data_struct() {
  initialized = sgp.begin();
  init_time   = millis();
}

#endif // ifdef USES_P083
