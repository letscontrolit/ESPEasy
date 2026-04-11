#include "../Helpers/CRC_functions.h"


int calc_CRC16(const String& text) {
  return calc_CRC16(text.c_str(), text.length());
}

int calc_CRC16(const char *ptr, int count)
{
  int crc;

  crc = 0;

  if (ptr == nullptr) {
    return crc;
  }

  while (--count >= 0)
  {
    crc = crc ^ static_cast<int>(*ptr++) << 8;
    char i = 8;

    do
    {
      if (crc & 0x8000) {
        crc = crc << 1 ^ 0x1021;
      }
      else {
        crc = crc << 1;
      }
    } while (--i);
  }
  return crc;
}

uint32_t calc_CRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;

  if (data != nullptr) {
    while (length--) {
      uint8_t c = *data++;

      for (uint32_t i = 0x80; i > 0; i >>= 1) {
        bool bit = crc & 0x80000000;

        if (c & i) {
          bit = !bit;
        }
        crc <<= 1;

        if (bit) {
          crc ^= 0x04c11db7;
        }
      }
    }
  }
  return crc;
}

uint8_t calc_CRC8(const uint8_t *data, size_t length)
{
  /*
   *	Name           : CRC-8
   * Polynomial     : 0x31 (x8 + x5 + x4 + 1)
   * Initialization : 0xFF
   * Reflect input  : False
   * Reflect output : False
   * Final          : XOR 0x00
   *	Example        : calc_CRC8( *(0xBE, 0xEF)) == 0x92 should be true
   */
  uint8_t crc = 0xFF;

  // for (uint8_t bytenr = 0; bytenr < 2; ++bytenr) {
  if (data != nullptr) {
    while (length--) {
      uint8_t c = *data++;
      crc ^= c;

      for (uint8_t i = 0; i < 8; ++i) {
        crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
      }
    }
  }
  return crc;
}

bool calc_CRC8(uint8_t MSB, uint8_t LSB, uint8_t CRC)
{
  /*
   *	Name           : CRC-8
   * Polynomial     : 0x31 (x8 + x5 + x4 + 1)
   * Initialization : 0xFF
   * Reflect input  : False
   * Reflect output : False
   * Final          : XOR 0x00
   *	Example        : CRC8( 0xBE, 0xEF, 0x92) should be true
   */
  uint8_t crc = 0xFF;

  for (uint8_t bytenr = 0; bytenr < 2; ++bytenr) {
    crc ^= (bytenr == 0) ? MSB : LSB;

    for (uint8_t i = 0; i < 8; ++i) {
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
  }
  return crc == CRC;
}
