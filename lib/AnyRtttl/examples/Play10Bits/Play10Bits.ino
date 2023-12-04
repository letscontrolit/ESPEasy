#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//The BitReader library is required for extracting 10 bit blocks from the RTTTL buffer.
//It can be installed from Arduino Library Manager or from https://github.com/end2endzone/BitReader/releases
#include <bitreader.h>

//project's constants
#define BUZZER_PIN 8

//RTTTL 10 bits binary format for the following: tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a
const unsigned char tetris10[] = {0x0A, 0x14, 0x12, 0xCE, 0x34, 0xE0, 0x82, 0x14, 0x32, 0x38, 0xE0, 0x4C, 0x2A, 0xAD, 0x34, 0xA0, 0x84, 0x0B, 0x0E, 0x28, 0xD3, 0x4C, 0x03, 0x2A, 0x28, 0xA1, 0x80, 0x2A, 0xA5, 0xB4, 0x93, 0x82, 0x1B, 0xAA, 0x38, 0xE2, 0x86, 0x12, 0x4E, 0x38, 0xA0, 0x84, 0x0B, 0x0E, 0x28, 0xD3, 0x4C, 0x03, 0x2A, 0x28, 0xA1, 0x80, 0x2A, 0xA9, 0x04};
const int tetris10_length = 42;

//bit reader support
#ifndef USE_BITADDRESS_READ_WRITE
BitReader bitreader;
#else
BitAddress bitreader;
#endif
uint16_t readNextBits(uint8_t numBits)
{
  uint16_t bits = 0;
  bitreader.read(numBits, &bits);
  return bits;
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  bitreader.setBuffer(tetris10);
  
  Serial.begin(115200);
  Serial.println();
}

void loop() {
  anyrtttl::blocking::play10Bits(BUZZER_PIN, tetris10_length, &readNextBits);

  while(true)
  {
  }
}
