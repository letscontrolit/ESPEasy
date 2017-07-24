#include "jkSDS011.h"

CjkSDS011::CjkSDS011(int16_t pinRX, int16_t pinTX) : _serial(pinRX, pinTX)
{
  _sws = ! ( pinRX < 0 || pinRX == 3 );
  _pm2_5 = NAN;
  _pm10_ = NAN;
  _available = false;
  _pm2_5avr = 0;
  _pm10_avr = 0;
  _avr = 0;
  _data.SetPacketLength(10);

  _serial.begin(9600);
}

void CjkSDS011::Process()
{
  while (_serial.available())
  {
  	_data.AddData(_serial.read());

    if (_data[0] == 0xAA && _data[9] == 0xAB && (_data[1] == 0xC0 || _data[1] == 0xCF))   // correct packet frame?
    {
      byte checksum = 0;
      for (byte i=2; i<= 7; i++)
      checksum += _data[i];
      if (checksum != _data[8])
        continue;

      if (_data[1] == 0xC0)   // SDS011 or SDS018?
      {
        _pm2_5 = (float)((_data[3] << 8) | _data[2]) * 0.1;
        _pm10_ = (float)((_data[5] << 8) | _data[4]) * 0.1;
        _available = true;
      }
      else if (_data[1] == 0xCF)   // SDS198?
      {
        _pm2_5 = (float)((_data[5] << 8) | _data[4]);
        _pm10_ = (float)((_data[3] << 8) | _data[2]);
        _available = true;
      }
      else
        continue;

      _pm2_5avr += _pm2_5;
      _pm10_avr += _pm10_;
      _avr++;
      return;
    }
  }
}

boolean CjkSDS011::available()
{
  boolean ret = _available;
  _available = false;
  return ret;
}

void CjkSDS011::ReadAverage(float &pm25, float &pm10)
{
  if (_avr)
  {
    pm25 = _pm2_5avr / _avr;
    pm10 = _pm10_avr / _avr;
    _pm2_5avr = 0;
    _pm10_avr = 0;
    _avr = 0;
  }
  else
  {
    pm25 = NAN;
    pm10 = NAN;
  }
}
