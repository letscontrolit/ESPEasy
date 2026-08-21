#ifndef AT24C04_H
#define AT24C04_H

#include "at24cxxx.h"

// Valid addresses for AT24C04
#define AT24C04_ADDRESS_0 (0x50)
#define AT24C04_ADDRESS_1 (0x52)
#define AT24C04_ADDRESS_2 (0x54)
#define AT24C04_ADDRESS_3 (0x56)

class AT24C04 : public AT24Cxxx {

  public:
    AT24C04(uint8_t address, TwoWire& i2c = Wire, uint8_t writeDelay = 6) :
      AT24Cxxx(address, i2c, writeDelay, 512, 16) {
    }

  protected:
    virtual void writeAddress(uint16_t address) override {
      // AT24C04 has 512 bytes, but only an 8-bit address parameter
      // The 9th bit is sent in the lsb of the chip i2c address leaving
      // only 2 bits (four addresses) to address chips.
      twoWire->beginTransmission((uint8_t)((i2cAddress & 0xFE) | ((address >> 8) & 0x1)));
      twoWire->write((uint8_t)(address & 0xFF));
    }

};

#endif
