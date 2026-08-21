#ifndef AT24C256_H
#define AT24C256_H

#include "at24cxxx.h"

class AT24C256 : public AT24Cxxx {

  public:
    AT24C256(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 32768, 64) {}
};

#endif
