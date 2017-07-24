#include "SensorSerialBuffer.h"

CSensorSerialBuffer::CSensorSerialBuffer()
{
  _writeIndex = 0;
  _packetLength = 0;
  Clear();
}

void CSensorSerialBuffer::Clear ()
{
  for (byte i=0; i<SERIALBUFFER_SIZE; i++)
    _buffer[i] = 0;
}

void CSensorSerialBuffer::AddData (byte b)
{
  _buffer[_writeIndex] = b;
  _writeIndex++;
  _writeIndex &= SERIALBUFFER_MASK;
}

void CSensorSerialBuffer::SetPacketLength (byte len)
{
  _packetLength = len;
}

byte& CSensorSerialBuffer::operator[] (byte x)
{
  x += _writeIndex;
  x -= _packetLength;
  x &= SERIALBUFFER_MASK;
  return _buffer[x];
}
