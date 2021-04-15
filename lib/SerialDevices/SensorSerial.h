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

#ifndef _SENSORSERIAL_H_
#define _SENSORSERIAL_H_

#include "Arduino.h"
#include <ESPeasySerial.h>

// FIXME TD-er  This object has now become obsolete.

class SensorSerial : public ESPeasySerial
{
public:
  SensorSerial(ESPEasySerialPort port, int receivePin, int transmitPin = -1, bool inverse_logic = false, unsigned int buffSize = 64);

  void begin(long speed);

  int peek();

  virtual size_t write(uint8_t byte);

  virtual int read();

  virtual int available();

  virtual void flush();

};

#endif
