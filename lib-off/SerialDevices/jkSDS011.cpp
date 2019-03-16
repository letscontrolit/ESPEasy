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
#ifdef ESP8266  // Needed for precompile issues.
#include "jkSDS011.h"

CjkSDS011::CjkSDS011(int16_t pinRX, int16_t pinTX)
{
  _sws = ! ( pinRX < 0 || pinRX == 3 );
  _pm2_5 = NAN;
  _pm10_ = NAN;
  _available = false;
  _pm2_5avr = 0;
  _pm10_avr = 0;
  _avr = 0;
  _data.SetPacketLength(10);
  _command.SetPacketLength(19);
  _working_period = -1;
  _sleepmode_active = false;
  _serial = new ESPeasySoftwareSerial(pinRX, pinTX);
  _serial->begin(9600);
}

CjkSDS011::~CjkSDS011() {
  delete _serial;
}

void CjkSDS011::SendCommand(byte byte1, byte byte2, byte byte3) {
  _command[0] = 0xAA; // Head
  _command[1] = 0xB4; // Command ID
  _command[2] = byte1; // Data Byte 1
  _command[3] = byte2; // Data Byte 2
  _command[4] = byte3; // Data Byte 3
  _command[5] = 0; // Data Byte 4
  _command[6] = 0; // Data Byte 5
  _command[7] = 0; // Data Byte 6
  _command[8] = 0; // Data Byte 7
  _command[9] = 0; // Data Byte 8
  _command[10] = 0; // Data Byte 9
  _command[11] = 0; // Data Byte 10
  _command[12] = 0; // Data Byte 11
  _command[13] = 0; // Data Byte 12
  _command[14] = 0; // Data Byte 13
  _command[15] = 0xFF; // Device ID byte 1, FF: All sensor response
  _command[16] = 0xFF; // Device ID byte 2, FF: All sensor response

  byte checksum = 0;
  for (byte i=2; i< 17; ++i) {
    checksum += _command[i];
  }
  _command[17] = checksum; // Checksum
  _command[18] = 0xAB; // Tail

  for (int i = 0; i < 19; ++i) {
    _serial->write(_command[i]);
  }
}

void CjkSDS011::SetSleepMode(bool enabled) {
  byte databyte3 = enabled ? 0 : 1;
  SendCommand(6, 1, databyte3);
}

void CjkSDS011::SetWorkingPeriod(int minutes) {
  // Data byte 1: 8
  // Data byte 2: 0: query the current mode
  //              1: set mode
  // Data byte 3: 0：continuous(default)
  //              1-30minute：work 30 seconds and sleep n*60-30 seconds
  if (minutes < 0 || minutes > 30) return;
  // Working period is stored in the flash of the sensor. Only write to change.
  const int currentWorkingPeriod = GetWorkingPeriod();
  if (minutes != currentWorkingPeriod)
    SendCommand(8, 1, minutes);
}

int CjkSDS011::GetWorkingPeriod() {
  // Data byte 1: 8
  // Data byte 2: 0: query the current mode
  //              1: set mode
  // Data byte 3: 0：continuous(default)
  //              1-30minute：work 30 seconds and sleep n*60-30 seconds
  SendCommand(8, 0, 0);
  Process();
  return _working_period;
}

void CjkSDS011::ParseCommandReply() {
  switch(_data[2]) {
    case 6: // Enable/Disable sleep mode.
      if (_data[3] == 0)
        _sleepmode_active = _data[4];
      break;
    case 8: // Set/Get working period
      if (_data[3] == 0)
        _working_period = _data[4];
      break;
    default:
      // Not implemented.
      break;
  }
}

void CjkSDS011::Process()
{
  while (_serial->available())
  {
  	_data.AddData(_serial->read());

    if (_data[0] == 0xAA && _data[9] == 0xAB)   // correct packet frame?
    {
      byte checksum = 0;
      for (byte i=2; i<= 7; i++)
        checksum += _data[i];
      if (checksum != _data[8])
        continue;

      switch(_data[1]) {
        case 0xC0:     // SDS011 or SDS018?
          _pm2_5 = (float)((_data[3] << 8) | _data[2]) * 0.1;
          _pm10_ = (float)((_data[5] << 8) | _data[4]) * 0.1;
          _available = true;
          break;
        case 0xCF:    // SDS198?
          _pm2_5 = (float)((_data[5] << 8) | _data[4]);
          _pm10_ = (float)((_data[3] << 8) | _data[2]);
          _available = true;
          break;
        case 0xC5:   // Reply on command ID 0xB4
          ParseCommandReply();
          break;
        default:
          break;
      }
      if (_available) {
        _pm2_5avr += _pm2_5;
        _pm10_avr += _pm10_;
        _avr++;
        _data.Clear();
        return;
      }
    }
  }
}

boolean CjkSDS011::available()
{
  boolean ret = _available;
  _available = false;
  return ret;
}

boolean CjkSDS011::ReadAverage(float &pm25, float &pm10)
{
  if (_avr)
  {
    pm25 = _pm2_5avr / _avr;
    pm10 = _pm10_avr / _avr;
    _pm2_5avr = 0;
    _pm10_avr = 0;
    _avr = 0;
    return true;
  }
  pm25 = NAN;
  pm10 = NAN;
  return false;
}
#endif
