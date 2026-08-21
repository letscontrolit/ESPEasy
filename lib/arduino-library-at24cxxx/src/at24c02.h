#ifndef AT24C02_H
#define AT24C02_H

#include "at24cxxx.h"

class AT24C02 : public AT24Cxxx {

  public:
    AT24C02(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 256, 8) {}

  protected:
    virtual void writeAddress(uint16_t address) override {
      twoWire->beginTransmission(i2cAddress);
      twoWire->write((uint8_t)(address & 0xFF));
    }

};

#endif
