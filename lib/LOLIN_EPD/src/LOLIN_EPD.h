#ifndef __LOLIN_EPD_H
#define __LOLIN_EPD_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(__SAM3X8E__)
typedef volatile RwReg PortReg; ///< a port register for fast access
typedef uint32_t PortMask;      ///< a port register mask for your pin
#define HAVE_PORTREG
#elif defined(ARDUINO_ARCH_SAMD)
// not supported
#elif defined(ESP8266) || defined(ESP32) || defined(ARDUINO_STM32_FEATHER) || defined(__arc__)
typedef volatile uint32_t PortReg; ///< a port register for fast access
typedef uint32_t PortMask;         ///< a port register mask for your pin
#elif defined(__AVR__)
typedef volatile uint8_t PortReg; ///< a port register for fast access
typedef uint8_t PortMask;         ///< a port register mask for your pin
#define HAVE_PORTREG
#else
// chances are its 32 bit so assume that
typedef volatile uint32_t PortReg; ///< a port register for fast access
typedef uint32_t PortMask;         ///< a port register mask for your pin
#endif

#include <SPI.h>
#include <Adafruit_GFX.h>

/**************************************************************************/
/*!
    available EPD colors
*/
/**************************************************************************/
enum
{
  EPD_BLACK,   ///< black color
  EPD_WHITE,   ///< white color
  EPD_INVERSE, ///< invert color
  EPD_RED,     ///< red color
  EPD_DARK,    ///< darker color
  EPD_LIGHT,   ///< lighter color
};

#define EPD_swap(a, b) \
  {                    \
    int16_t t = a;     \
    a = b;             \
    b = t;             \
  } ///< simple swap function

/**************************************************************************/
/*!
    Class for interfacing with LOLIN EPD display breakouts.
*/
/**************************************************************************/

class LOLIN_EPD: public Adafruit_GFX
{
public:
  LOLIN_EPD(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY = -1);
  LOLIN_EPD(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY = -1);

  ~LOLIN_EPD();
  void begin(bool reset = true);

protected:
  int8_t sid, ///< sid pin
      sclk,   ///< serial clock pin
      dc,     ///< data/command pin
      rst,    ///< reset pin
      cs,     ///< chip select pin
      busy;   ///< busy pin

  bool blackInverted,  ///< is black channel inverted
      redInverted;     ///< is red channel inverted
  int bw_bufsize,      ///< size of the black and white buffer
      red_bufsize;     ///< size of the red buffer
  bool singleByteTxns; ///< if true CS will go high after every data byte transferred

  uint8_t *bw_buf;  ///< the pointer to the black and white buffer if using on-chip ram
  uint8_t *red_buf; ///< the pointer to the red buffer if using on-chip ram

  void sendCmd(uint8_t c);
  void sendData(uint8_t data);
  uint8_t fastSPIwrite(uint8_t c);

  boolean hwSPI; ///< true if using hardware SPI
#ifdef HAVE_PORTREG
  PortReg *mosiport,    ///< mosi port register
      *clkport,         ///< serial clock port register
      *csport,          ///< chip select port register
      *dcport;          ///< data/command port register
  PortMask mosipinmask, ///< mosi pin mask
      clkpinmask,       ///< serial clock pin mask
      cspinmask,        ///< chip select pin mask
      dcpinmask;        ///< data / command pin mask
#endif
  void csLow();
  void csHigh();
  void dcHigh();
  void dcLow();

private:
};

#include "LOLIN_IL3897.h"

#endif
