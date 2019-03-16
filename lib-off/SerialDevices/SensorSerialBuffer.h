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
