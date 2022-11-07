#include <Arduino.h>
#include <IRSender.h>

// The generic functions of the abstract IRSender class

IRSender::IRSender(uint8_t pin)
{
  _pin = pin;
}


// Send a uint8_t (8 bits) over IR
void IRSender::sendIRbyte(uint8_t sendByte, int bitMarkLength, int zeroSpaceLength, int oneSpaceLength)
{
  for (int i=0; i<8 ; i++)
  {
    if (sendByte & 0x01)
    {
      mark(bitMarkLength);
      space(oneSpaceLength);
    }
    else
    {
      mark(bitMarkLength);
      space(zeroSpaceLength);
    }

    sendByte >>= 1;
  }
}


// The Carrier IR protocol has the bits in a reverse order (compared to the other heatpumps)
// See http://www.nrtm.org/index.php/2013/07/25/reverse-bits-in-a-uint8_t/
uint8_t IRSender::bitReverse(uint8_t x)
{
  //          01010101  |         10101010
  x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
  //          00110011  |         11001100
  x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
  //          00001111  |         11110000
  x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
  return x;
}


// Definitions of virtual functions
void IRSender::setFrequency(int) {};
void IRSender::space(int) {};
void IRSender::mark(int) {};