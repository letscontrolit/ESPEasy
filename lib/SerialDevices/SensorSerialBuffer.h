#ifndef _SENSORSERIALBUFFER_H_
#define _SENSORSERIALBUFFER_H_

#include "Arduino.h"

#define SERIALBUFFER_SIZE 32
#define SERIALBUFFER_MASK 31

class CSensorSerialBuffer
{
public:
  CSensorSerialBuffer();

  void Clear ();

  void AddData (byte b);

  void SetPacketLength (byte len);

  byte& operator[] (byte x);

private:
  byte _buffer[SERIALBUFFER_SIZE];
  byte _writeIndex;
  byte _packetLength;
};

#endif
