/*-------------------------------------------------------------------------
  Arduino library to ...

  Written by Jochen Krapf,
  contributions by ... and other members of the open
  source community.

  -------------------------------------------------------------------------
  This file is part of the MechInputs library.

  MechInputs is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  MechInputs is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with MechInputs.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

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
