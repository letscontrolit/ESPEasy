#include "P062_data_struct.h"


#ifdef USES_P062

P062_data_struct::P062_data_struct(uint8_t i2c_addr, bool scancode) : use_scancode(scancode) {
  keypad.begin(i2c_addr);
}

bool P062_data_struct::readKey(uint16_t& key) {
  key = keypad.touched();

  if (key && use_scancode)
  {
    uint16_t colMask = 0x01;

    for (byte col = 1; col <= 12; col++)
    {
      if (key & colMask) // this key pressed?
      {
        key = col;
        break;
      }
      colMask <<= 1;
    }
  }

  if (keyLast != key)
  {
    keyLast = key;
    return true;
  }
  return false;
}

#endif // ifdef USES_P062
