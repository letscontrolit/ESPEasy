#ifndef _jkSDS011_H_
#define _jkSDS011_H_

#include "Arduino.h"
#include "SensorSerial.h"
#include "SensorSerialBuffer.h"

class CjkSDS011
{
public:
  CjkSDS011(int16_t pinRX, int16_t pinTX);

  void Process();

  boolean available();

  float GetPM2_5() { return _pm2_5; };
  float GetPM10_() { return _pm10_; };

  void ReadAverage(float &pm25, float &pm10);

private:
  SensorSerial _serial;
  CSensorSerialBuffer _data;
  float _pm2_5;
  float _pm10_;
  float _pm2_5avr;
  float _pm10_avr;
  uint16_t _avr;
  boolean _available;
  boolean _sws;
};

#endif
