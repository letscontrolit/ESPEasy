#include "P057_data_struct.h"


#ifdef USES_P057

P057_data_struct::P057_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {
  ledMatrix.Init(i2cAddress);
}

#endif // ifdef USES_P057
