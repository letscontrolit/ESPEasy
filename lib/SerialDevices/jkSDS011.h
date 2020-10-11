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
//#ifdef ESP8266  // Needed for precompile issues.
#ifndef _jkSDS011_H_
#define _jkSDS011_H_

#include "Arduino.h"
//#include "SensorSerial.h"
#include "SensorSerialBuffer.h"
#include "ESPeasySerial.h"


class CjkSDS011
{
public:
  CjkSDS011(ESPEasySerialPort port, int16_t pinRX, int16_t pinTX);
  virtual ~CjkSDS011();

  void Process();

  boolean available();

  float GetPM2_5() { return _pm2_5; };
  float GetPM10_() { return _pm10_; };

  // Return true when there are valid samples.
  boolean ReadAverage(float &pm25, float &pm10);

  void SetSleepMode(bool enabled);

  // Set interval to get new data, 0 .. 30 minutes.
  // Minutes = 0 => continous mode.
  // The setting is still effective after power off(factory default is continuous measurement)
  void SetWorkingPeriod(int minutes);

  // Get the working period in minutes (0 = continuous reading)
  // Negative return value indicates error during communication.
  int GetWorkingPeriod();

private:
  void SendCommand(byte byte1, byte byte2, byte byte3);
  void ParseCommandReply();

//  SensorSerial _serial;
  ESPeasySerial *_serial = nullptr;
  CSensorSerialBuffer _data;
  CSensorSerialBuffer _command;
  float _pm2_5;
  float _pm10_;
  float _pm2_5avr;
  float _pm10_avr;
  uint16_t _avr;
  boolean _available;
  boolean _sws;
  int _working_period;
  boolean _sleepmode_active;
};

#endif
//#endif
