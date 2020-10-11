#include "HT16K33.h"
#include <Wire.h>

CHT16K33::CHT16K33(void)
{
  
};

void CHT16K33::Init(uint8_t addr)
{
  _addr = addr;

  // System Setup Register
  Wire.beginTransmission(_addr);
  Wire.write(0x21);  // oscillator on
  Wire.endTransmission();

  // Display Setup Register
  Wire.beginTransmission(_addr);
  Wire.write(0x81);  // blink off; display on
  Wire.endTransmission();

  SetBrightness(15);
  ClearRowBuffer();
  TransmitRowBuffer();
};

void CHT16K33::SetBrightness(uint8_t b)
{
  if (b > 15)
    b = 15;
  // Digital Dimming Data Input
  Wire.beginTransmission(_addr);
  Wire.write(0xE0 | b);  // brightness
  Wire.endTransmission();
};

void CHT16K33::TransmitRowBuffer(void)
{
  // Display Memory
  Wire.beginTransmission(_addr);
  Wire.write(0); // start data at address 0
  for (byte i=0; i<8; i++)
  {
    Wire.write(_rowBuffer[i] & 0xFF);
    Wire.write(_rowBuffer[i] >> 8);
  }
  Wire.endTransmission();
};

void CHT16K33::ClearRowBuffer(void)
{
  for (byte i=0; i<8; i++)
    _rowBuffer[i] = 0;
};

void CHT16K33::SetRow(uint8_t com, uint16_t data)
{
  if (com < 8)
    _rowBuffer[com] = data;
};

uint16_t CHT16K33::GetRow(uint8_t com)
{
  if (com < 8)
    return _rowBuffer[com];
  else
    return 0;
};

void CHT16K33::SetDigit(uint8_t com, uint8_t c)
{
  uint16_t value = 0;

  if (c <= 0xF)
    value = _digits[c];

  switch (c)
  {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      value = c + 10 - 'A';
      value = _digits[value & 0xF];
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      value = c - '0';
      value = _digits[value & 0xF];
      break;
    case ' ':
      value = 0;
      break;
    case ':':
      value = 0x02;   //special for China 4 x 7-seg big display
      break;
    case '-':
      value = 0x40;
      break;
  }

  SetRow(com, value);
}

uint8_t CHT16K33::ReadKeys(void)
{
  // Display Memory
  Wire.beginTransmission(_addr);
  Wire.write(0x40); // start data at address 0x40
  Wire.endTransmission();

  Wire.requestFrom(_addr, (uint8_t)6);
  if (Wire.available() == 6)
  {
    for (byte i=0; i<3; i++)
    {
      _keyBuffer[i] = Wire.read() | (Wire.read() << 8);
    }
    // Not needed See: https://github.com/Koepel/How-to-use-the-Arduino-Wire-library/wiki/Common-mistakes#2
    // Wire.endTransmission();
  }

  for (byte i=0; i<3; i++)
  {
    byte mask = 1;
    for (byte k=0; k<12; k++)
    {
      if (_keyBuffer[i] & mask)
      {
        _keydown = 16*(i+1) + (k+1);
        return _keydown;
      }
      mask <<= 1;
    }
  }
  _keydown = 0;
  return _keydown;
};

const uint8_t CHT16K33::_digits[16] =
{
  0x3F,   // 0
  0x06,   // 1
  0x5B,   // 2
  0x4F,   // 3
  0x66,   // 4
  0x6D,   // 5
  0x7D,   // 6
  0x07,   // 7
  0x7F,   // 8
  0x6F,   // 9
  0x77,   // A
  0x7C,   // B
  0x39,   // C
  0x5E,   // D
  0x79,   // E
  0x71,   // F
};
