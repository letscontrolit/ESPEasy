/*
 * Adafruit_ST7796S_kbv class inherits from Adafruit_GFX, Adafruit_SPITFT class and the Arduino Print class.
 * Adafruit_ST7796S_kbv written by David Prentice
 *
 * Any use of Adafruit_ST7796S_kbv class and examples is dependent on Adafruit and Arduino licenses
 * The license texts are in the accompanying license.txt file
 */

/*!
 * @file Adafruit_ST7796S_kbv.h
 *
 * These displays use SPI to communicate, 4 or 5 pins are required
 * to interface (RST is optional IF YOU ADD A PULLUP RESISTOR).
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
 * Adafruit_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * Adafruit_GFX, Adafruit_SPITFT written by Limor "ladyada" Fried for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */


#ifndef _ADAFRUIT_ST7796S_KBV_H_
#define _ADAFRUIT_ST7796S_KBV_H_

#include "Adafruit_GFX.h"
#include "Arduino.h"
#include "Print.h"
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <SPI.h>
#include "Adafruit_ST77xx.h"

#define ST7796S_TFTWIDTH 320    ///< ST7796S max TFT width
#define ST7796S_TFTHEIGHT 480   ///< ST7796S max TFT height

#define ST7796S_NOP 0x00        ///< No-op register
#define ST7796S_SWRESET 0x01    ///< Software reset register
#define ST7796S_RDDID 0x04      ///< Read display identification information
#define ST7796S_RDDST 0x09      ///< Read Display Status

#define ST7796S_SLPIN 0x10      ///< Enter Sleep Mode
#define ST7796S_SLPOUT 0x11     ///< Sleep Out
#define ST7796S_PTLON 0x12      ///< Partial Mode ON
#define ST7796S_NORON 0x13      ///< Normal Display Mode ON

#define ST7796S_RDMODE 0x0A     ///< Read Display Power Mode
#define ST7796S_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ST7796S_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ST7796S_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ST7796S_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ST7796S_INVOFF 0x20     ///< Display Inversion OFF
#define ST7796S_INVON 0x21      ///< Display Inversion ON
#define ST7796S_GAMMASET 0x26   ///< Gamma Set
#define ST7796S_DISPOFF 0x28    ///< Display OFF
#define ST7796S_DISPON 0x29     ///< Display ON

#define ST7796S_CASET 0x2A      ///< Column Address Set
#define ST7796S_PASET 0x2B      ///< Page Address Set
#define ST7796S_RAMWR 0x2C      ///< Memory Write
#define ST7796S_RAMRD 0x2E      ///< Memory Read

#define ST7796S_PTLAR 0x30      ///< Partial Area
#define ST7796S_VSCRDEF 0x33    ///< Vertical Scrolling Definition
#define ST7796S_MADCTL 0x36     ///< Memory Access Control
#define ST7796S_VSCRSADD 0x37   ///< Vertical Scrolling Start Address
#define ST7796S_PIXFMT 0x3A     ///< COLMOD: Pixel Format Set


// Color definitions
#define ST7796S_BLACK 0x0000       ///<   0,   0,   0
#define ST7796S_NAVY 0x000F        ///<   0,   0, 123
#define ST7796S_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ST7796S_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ST7796S_MAROON 0x7800      ///< 123,   0,   0
#define ST7796S_PURPLE 0x780F      ///< 123,   0, 123
#define ST7796S_OLIVE 0x7BE0       ///< 123, 125,   0
#define ST7796S_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ST7796S_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ST7796S_BLUE 0x001F        ///<   0,   0, 255
#define ST7796S_GREEN 0x07E0       ///<   0, 255,   0
#define ST7796S_CYAN 0x07FF        ///<   0, 255, 255
#define ST7796S_RED 0xF800         ///< 255,   0,   0
#define ST7796S_MAGENTA 0xF81F     ///< 255,   0, 255
#define ST7796S_YELLOW 0xFFE0      ///< 255, 255,   0
#define ST7796S_WHITE 0xFFFF       ///< 255, 255, 255
#define ST7796S_ORANGE 0xFD20      ///< 255, 165,   0
#define ST7796S_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ST7796S_PINK 0xFC18        ///< 255, 130, 198

/**************************************************************************/

/*!
   @brief Class to manage hardware interface with ST7796S chipset
 */

/**************************************************************************/

class Adafruit_ST7796S_kbv : public Adafruit_ST77xx {
public:

  Adafruit_ST7796S_kbv(int8_t _CS,
                       int8_t _DC,
                       int8_t _MOSI,
                       int8_t _SCLK,
                       int8_t _RST  = -1,
                       int8_t _MISO = -1);
  Adafruit_ST7796S_kbv(int8_t _CS,
                       int8_t _DC,
                       int8_t _RST = -1);
#if !defined(ESP8266)
  Adafruit_ST7796S_kbv(SPIClass *spiClass,
                       int8_t    dc,
                       int8_t    cs  = -1,
                       int8_t    rst = -1);
#endif // end !ESP8266
  // Adafruit_ST7796S_kbv(tftBusWidth busWidth,
  //                      int8_t      d0,
  //                      int8_t      wr,
  //                      int8_t      dc,
  //                      int8_t      cs  = -1,
  //                      int8_t      rst = -1,
  //                      int8_t      rd  = -1);

  void begin(uint32_t freq = 0);
  void setRotation(uint8_t r);
  void invertDisplay(bool i);
  void scrollTo(uint16_t y);
  void setScrollMargins(uint16_t top,
                        uint16_t bottom);

  // Transaction API not used by GFX
  void setAddrWindow(uint16_t x,
                     uint16_t y,
                     uint16_t w,
                     uint16_t h);

  uint8_t readcommand8(uint8_t reg,
                       uint8_t index = 0);
};

#endif // _ADAFRUIT_ST7796SH_
