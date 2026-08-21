#ifndef AT24C64_H
#define AT24C64_H

#include "at24cxxx.h"

class AT24C64 : public AT24Cxxx {

  public:
    AT24C64(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 8192, 32) {}
};

#endif
