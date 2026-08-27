
#ifndef AT24CXXX_H
#define AT24CXXX_H

/**
 * 2026-08-19 tonhuisman: Updated to allow use of AT24C512, AT24C1024 and AT24C2048 by changing uint16_t argments to size_t
 */
#include "Wire.h"

#define AT24C_ADDRESS_0 (0x50)
#define AT24C_ADDRESS_1 (0x51)
#define AT24C_ADDRESS_2 (0x52)
#define AT24C_ADDRESS_3 (0x53)
#define AT24C_ADDRESS_4 (0x54)
#define AT24C_ADDRESS_5 (0x55)
#define AT24C_ADDRESS_6 (0x56)
#define AT24C_ADDRESS_7 (0x57)

class AT24Cxxx {

  public:
    AT24Cxxx(uint8_t address, TwoWire& i2c, int writeDelay, size_t size, uint8_t pageSize);
    uint8_t read( int idx );
    void write( int idx, uint8_t val);
    void update( int idx, uint8_t val);
    size_t length();
    int writeBuffer(size_t address, const uint8_t* data, size_t len);
    int readBuffer(size_t address, uint8_t* data, size_t len);
    /** 
     * Returns result from the last performed operation. 
     * The meaning of the values are:
     *  0 .. success
     *  1 .. length to long for buffer
     *  2 .. address send, NACK received - typically means no device at the address
     *  3 .. data send, NACK received
     *  4 .. other twi error (lost bus arbitration, bus error, ..)
     */
    uint8_t getLastError();

    template< typename T > T &get( int idx, T &t ){
      readBuffer(idx, (uint8_t*)&t, sizeof(T));
      return t;
    }

    template< typename T > const T &put( int idx, const T &t ){
      writeBuffer(idx, (uint8_t*)&t, sizeof(T));
      return t;
    }

  protected:
    virtual void writeAddress(uint16_t address);
    uint8_t i2cAddress;
    TwoWire* twoWire;
    
  private:
    int rawWriteBuffer(size_t address, const uint8_t* data, size_t len);
    size_t size;
    uint8_t writeDelay;
    uint8_t lastError;
    uint8_t pageSize;
};

#endif
