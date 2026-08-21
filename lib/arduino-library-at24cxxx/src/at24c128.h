#ifndef AT24C128_H
#define AT24C128_H

#include "at24cxxx.h"

class AT24C128 : public AT24Cxxx {

  public:
    AT24C128(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 16384, 64) {}
};

#endif
