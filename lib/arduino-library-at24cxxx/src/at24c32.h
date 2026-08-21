#ifndef AT24C32_H
#define AT24C32_H

#include "at24cxxx.h"

class AT24C32 : public AT24Cxxx {

  public:
    AT24C32(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 4096, 32) {}
};

#endif
