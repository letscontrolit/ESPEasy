#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr) (*(const unsigned char *)(addr)) ///< read bytes from program memory
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
#include <util/delay.h>
#endif

#include <stdlib.h>

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "LOLIN_EPD.h"

LOLIN_EPD::LOLIN_EPD(int width, int height, int8_t spi_mosi, int8_t spi_clock, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY):Adafruit_GFX(width, height)
{
  cs = CS;
  rst = RST;
  dc = DC;
  sclk = spi_clock;
  sid = spi_mosi;
  busy = BUSY;
  hwSPI = false;
  singleByteTxns = false;
}

LOLIN_EPD::LOLIN_EPD(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY):Adafruit_GFX(width, height)
{

  dc = DC;
  rst = RST;
  cs = CS;
  busy = BUSY;
  hwSPI = true;
  singleByteTxns = false;
}

/**************************************************************************/
/*!
    @brief default destructor
*/
/**************************************************************************/
LOLIN_EPD::~LOLIN_EPD()
{

  free(bw_buf);
  free(red_buf);
}

/**************************************************************************/
/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
*/
/**************************************************************************/
void LOLIN_EPD::begin(bool reset)
{
  blackInverted = true;
  redInverted = false;

  // set pin directions
  pinMode(dc, OUTPUT);
  pinMode(cs, OUTPUT);
#ifdef HAVE_PORTREG
  csport = portOutputRegister(digitalPinToPort(cs));
  cspinmask = digitalPinToBitMask(cs);
  dcport = portOutputRegister(digitalPinToPort(dc));
  dcpinmask = digitalPinToBitMask(dc);
#endif

  csHigh();

  if (!hwSPI)
  {
    // set pins for software-SPI
    pinMode(sid, OUTPUT);
    pinMode(sclk, OUTPUT);
#ifdef HAVE_PORTREG
    clkport = portOutputRegister(digitalPinToPort(sclk));
    clkpinmask = digitalPinToBitMask(sclk);
    mosiport = portOutputRegister(digitalPinToPort(sid));
    mosipinmask = digitalPinToBitMask(sid);
#endif
  }
  else
  {
    SPI.begin();
#ifndef SPI_HAS_TRANSACTION
    SPI.setClockDivider(4);
#endif
  }

  if ((reset) && (rst >= 0))
  {
    // Setup reset pin direction
    pinMode(rst, OUTPUT);
    // VDD (3.3V) goes high at start, lets just chill for a ms
    digitalWrite(rst, HIGH);
    delay(1);
    // bring reset low
    digitalWrite(rst, LOW);
    // wait 10ms
    delay(10);
    // bring out of reset
    digitalWrite(rst, HIGH);
  }

  if (busy >= 0)
  {
    pinMode(busy, INPUT);
  }
}

/**************************************************************************/
/*!
    @brief send an EPD command with no data
    @param c the command to send
    @param end if true the cs pin will be pulled high following the transaction. If false the cs pin will remain low.
    @returns the data byte read over the SPI bus
*/
/**************************************************************************/
void LOLIN_EPD::sendCmd(uint8_t c)
{
  // SPI
  csHigh();
  dcLow();
  csLow();

  fastSPIwrite(c);

  csHigh();
}

/**************************************************************************/
/*!
    @brief send data to the display
    @param buf the data buffer to send
    @param len the length of the data buffer
*/
/**************************************************************************/
void LOLIN_EPD::sendData(uint8_t data)
{
  // SPI
  csHigh();
  dcHigh();
  csLow();

  fastSPIwrite(data);

  csHigh();
}

/**************************************************************************/
/*!
    @brief transfer a single byte over SPI.
    @param d the data to send
    @returns the data byte read
*/
/**************************************************************************/
uint8_t LOLIN_EPD::fastSPIwrite(uint8_t d)
{
  if (hwSPI)
  {
    if (singleByteTxns)
    {
      uint8_t b;
      csLow();
      b = SPI.transfer(d);
      csHigh();
      return b;
    }
    else
      return SPI.transfer(d);
  }
  else
  {
    //TODO: return read data for software SPI
    for (uint8_t bit = 0x80; bit; bit >>= 1)
    {
#ifdef HAVE_PORTREG
      *clkport &= ~clkpinmask;
      if (d & bit)
        *mosiport |= mosipinmask;
      else
        *mosiport &= ~mosipinmask;
      *clkport |= clkpinmask;
#else
      digitalWrite(sclk, LOW);
      if (d & bit)
        digitalWrite(sid, HIGH);
      else
        digitalWrite(sid, LOW);
      digitalWrite(sclk, HIGH);
#endif
    }
    return 0;
  }
}

/**************************************************************************/
/*!
    @brief set chip select pin high
*/
/**************************************************************************/
void LOLIN_EPD::csHigh()
{
#ifdef SPI_HAS_TRANSACTION
  SPI.endTransaction();
#endif
#ifdef HAVE_PORTREG
  *csport |= cspinmask;
#else
  digitalWrite(cs, HIGH);
#endif
}

/**************************************************************************/
/*!
    @brief set chip select pin low
*/
/**************************************************************************/
void LOLIN_EPD::csLow()
{
#ifdef SPI_HAS_TRANSACTION
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
#ifdef HAVE_PORTREG
  *csport &= ~cspinmask;
#else
  digitalWrite(cs, LOW);
#endif
}

/**************************************************************************/
/*!
    @brief set data/command pin high
*/
/**************************************************************************/
void LOLIN_EPD::dcHigh()
{
#ifdef HAVE_PORTREG
  *dcport |= dcpinmask;
#else
  digitalWrite(dc, HIGH);
#endif
}

/**************************************************************************/
/*!
    @brief set data/command pin low
*/
/**************************************************************************/
void LOLIN_EPD::dcLow()
{
#ifdef HAVE_PORTREG
  *dcport &= ~dcpinmask;
#else
  digitalWrite(dc, LOW);
#endif
}