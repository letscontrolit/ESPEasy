/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#ifndef _ADAFRUIT_ST77XXH_
#define _ADAFRUIT_ST77XXH_

#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#define ST7735_TFTWIDTH_128 128  // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80    // for mini
#define ST7735_TFTWIDTH_135 135
#define ST7735_TFTWIDTH_172 172
#define ST7735_TFTWIDTH_170 170
#define ST7735_TFTHEIGHT_128 128 // for 1.44" display
#define ST7735_TFTHEIGHT_160 160 // for 1.8" and mini display
#define ST7735_TFTHEIGHT_240 240
#define ST7735_TFTHEIGHT_320 320

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_RAMCTRL		0xB0      // RAM control
#define ST77XX_RGBCTRL		0xB1      // RGB control
#define ST77XX_PORCTRL		0xB2      // Porch control
#define ST77XX_FRCTRL1		0xB3      // Frame rate control
#define ST77XX_PARCTRL		0xB5      // Partial mode control
#define ST77XX_GCTRL		  0xB7      // Gate control
#define ST77XX_GTADJ		  0xB8      // Gate on timing adjustment
#define ST77XX_DGMEN		  0xBA      // Digital gamma enable
#define ST77XX_VCOMS		  0xBB      // VCOMS setting
#define ST77XX_LCMCTRL		0xC0      // LCM control
#define ST77XX_IDSET		  0xC1      // ID setting
#define ST77XX_VDVVRHEN		0xC2      // VDV and VRH command enable
#define ST77XX_VRHS			  0xC3      // VRH set
#define ST77XX_VDVSET		  0xC4      // VDV setting
#define ST77XX_VCMOFSET		0xC5      // VCOMS offset set
#define ST77XX_FRCTR2		  0xC6      // FR Control 2
#define ST77XX_CABCCTRL		0xC7      // CABC control
#define ST77XX_REGSEL1		0xC8      // Register value section 1
#define ST77XX_REGSEL2		0xCA      // Register value section 2
#define ST77XX_PWMFRSEL		0xCC      // PWM frequency selection
#define ST77XX_PWCTRL1		0xD0      // Power control 1
#define ST77XX_VAPVANEN		0xD2      // Enable VAP/VAN signal output
#define ST77XX_CMD2EN		  0xDF      // Command 2 enable
#define ST77XX_PVGAMCTRL	0xE0      // Positive voltage gamma control
#define ST77XX_NVGAMCTRL	0xE1      // Negative voltage gamma control
#define ST77XX_DGMLUTR		0xE2      // Digital gamma look-up table for red
#define ST77XX_DGMLUTB		0xE3      // Digital gamma look-up table for blue
#define ST77XX_GATECTRL		0xE4      // Gate control
#define ST77XX_SPI2EN		  0xE7      // SPI2 enable
#define ST77XX_PWCTRL2		0xE8      // Power control 2
#define ST77XX_EQCTRL		  0xE9      // Equalize time control
#define ST77XX_PROMCTRL		0xEC      // Program control
#define ST77XX_PROMEN		  0xFA      // Program mode enable
#define ST77XX_NVMSET		  0xFC      // NVM setting
#define ST77XX_PROMACT		0xFE      // Program action

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00
#define ST77XX_MADCTL_BGR 0x08

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

/// Subclass of SPITFT for ST77xx displays (lots in common!)
class Adafruit_ST77xx : public Adafruit_SPITFT {
public:
  Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t _CS, int8_t _DC, int8_t _MOSI,
                  int8_t _SCLK, int8_t _RST = -1, int8_t _MISO = -1);
  Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t CS, int8_t RS,
                  int8_t RST = -1);
#if !defined(ESP8266)
  Adafruit_ST77xx(uint16_t w, uint16_t h, SPIClass *spiClass, int8_t CS,
                  int8_t RS, int8_t RST = -1);
#endif // end !ESP8266
  virtual ~Adafruit_ST77xx();

  virtual void setRotation(uint8_t r);

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void enableDisplay(boolean enable);
  void enableTearing(boolean enable);
  void enableSleep(boolean enable);

protected:
  uint8_t _colstart = 0,   ///< Some displays need this changed to offset
      _rowstart = 0,       ///< Some displays need this changed to offset
      spiMode = SPI_MODE0; ///< Certain display needs MODE3 instead

  void begin(uint32_t freq = 0);
  void commonInit(const uint8_t *cmdList);
  void displayInit(const uint8_t *addr);
  void setColRowStart(int8_t col, int8_t row);
};

#endif // _ADAFRUIT_ST77XXH_
