#include "P058_data_struct.h"


#ifdef USES_P058

P058_data_struct::P058_data_struct(uint8_t i2c_addr) {
  keyPad.Init(i2c_addr);
}

bool P058_data_struct::readKey(uint8_t& key) {
  key = keyPad.ReadKeys();

  if (keyLast != key)
  {
    keyLast = key;
    return true;
  }
  return false;
}

#endif // ifdef USES_P058
