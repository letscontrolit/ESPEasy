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

#include <SensorSerial.h>

#ifdef ESP32
SensorSerial::SensorSerial(ESPEasySerialPort port, int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) :
 ESPeasySerial(port, receivePin, transmitPin, inverse_logic)
{}
#endif

#ifdef ESP8266
SensorSerial::SensorSerial(ESPEasySerialPort port, int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) :
 ESPeasySerial(port, receivePin, transmitPin, inverse_logic, buffSize)
{}
#endif

void SensorSerial::begin(long speed) {
  ESPeasySerial::begin(speed);
}

int SensorSerial::peek()
{
  return ESPeasySerial::peek();
}

size_t SensorSerial::write(uint8_t val)
{
  return ESPeasySerial::write(val);
}

int SensorSerial::read()
{
  return ESPeasySerial::read();
}

int SensorSerial::available()
{
  return ESPeasySerial::available();
}

void SensorSerial::flush()
{
  ESPeasySerial::flush();
}
