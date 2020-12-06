#include "../PluginStructs/P057_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <HT16K33.h>

#ifdef USES_P057

P057_data_struct::P057_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {
  ledMatrix.Init(i2cAddress);
}

#endif // ifdef USES_P057
