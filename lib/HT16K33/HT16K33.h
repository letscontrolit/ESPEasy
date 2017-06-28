#ifndef CHT16K33_h
#define CHT16K33_h

#include <Arduino.h>

class CHT16K33 {
 public:
  CHT16K33(void);

  void Init(uint8_t addr);

  // LED buffer
  void ClearRowBuffer(void);
  void SetRow(uint8_t com, uint16_t data);
  uint16_t GetRow(uint8_t com);
  void SetDigit(uint8_t com, uint8_t c);

  // LED output and control
  void SetBrightness(uint8_t b);
  void TransmitRowBuffer(void);

  //KeyPad Scan
  uint8_t ReadKeys(void);

protected:
  uint8_t _addr;
  uint16_t _rowBuffer[8];
  uint16_t _keyBuffer[3];
  byte _keydown;

  static const uint8_t _digits[16];
};

#endif
