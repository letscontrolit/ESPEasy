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


SensorSerial::SensorSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) : ESPeasySoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize)
{
  //boolean sws = isValidGPIOpin(receivePin);
  _hw = receivePin < 0;
}

void SensorSerial::begin(long speed)
{
  if (_hw)
    Serial.begin(speed);
  else
    ESPeasySoftwareSerial::begin(speed);
}

int SensorSerial::peek()
{
  if (_hw)
    return Serial.peek();
  else
    return ESPeasySoftwareSerial::peek();
}

size_t SensorSerial::write(uint8_t byte)
{
  if (_hw)
    return Serial.write(byte);
  else
    return ESPeasySoftwareSerial::write(byte);
}

int SensorSerial::read()
{
  if (_hw)
    return Serial.read();
  else
    return ESPeasySoftwareSerial::read();
}

int SensorSerial::available()
{
  if (_hw)
    return Serial.available();
  else
    return ESPeasySoftwareSerial::available();
}

void SensorSerial::flush()
{
  if (_hw)
    Serial.flush();
  else
    ESPeasySoftwareSerial::flush();
}
