/**************************************************************************/
/*!
  @file Adafruit_PCD8544.h

  This is a library for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

  These displays use SPI to communicate, 4 or 5 pins are required to
  interface

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution
*/
/**************************************************************************/
#ifndef _ADAFRUIT_PCD8544_H
#define _ADAFRUIT_PCD8544_H

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

#define LCDWIDTH 84  ///< LCD is 84 pixels wide
#define LCDHEIGHT 48 ///< 48 pixels high

#define PCD8544_POWERDOWN 0x04 ///< Function set, Power down mode
#define PCD8544_ENTRYMODE 0x02 ///< Function set, Entry mode
#define PCD8544_EXTENDEDINSTRUCTION                                            \
  0x01 ///< Function set, Extended instruction set control

#define PCD8544_DISPLAYBLANK 0x0    ///< Display control, blank
#define PCD8544_DISPLAYNORMAL 0x4   ///< Display control, normal mode
#define PCD8544_DISPLAYALLON 0x1    ///< Display control, all segments on
#define PCD8544_DISPLAYINVERTED 0x5 ///< Display control, inverse mode

#define PCD8544_FUNCTIONSET 0x20 ///< Basic instruction set
#define PCD8544_DISPLAYCONTROL                                                 \
  0x08 ///< Basic instruction set - Set display configuration
#define PCD8544_SETYADDR                                                       \
  0x40 ///< Basic instruction set - Set Y address of RAM, 0 <= Y <= 5
#define PCD8544_SETXADDR                                                       \
  0x80 ///< Basic instruction set - Set X address of RAM, 0 <= X <= 83

#define PCD8544_SETTEMP                                                        \
  0x04 ///< Extended instruction set - Set temperature coefficient
#define PCD8544_SETBIAS 0x10 ///< Extended instruction set - Set bias system
#define PCD8544_SETVOP                                                         \
  0x80 ///< Extended instruction set - Write Vop to register

/**************************************************************************/
/*!
    @brief The PCD8544 LCD class
 */
class Adafruit_PCD8544 : public Adafruit_GFX {
public:
  Adafruit_PCD8544(int8_t sclk_pin, int8_t din_pin, int8_t dc_pin,
                   int8_t cs_pin, int8_t rst_pin);
  Adafruit_PCD8544(int8_t dc_pin, int8_t cs_pin, int8_t rst_pin,
                   SPIClass *theSPI = &SPI);

  virtual ~Adafruit_PCD8544() {}

  bool begin(uint8_t contrast = 40, uint8_t bias = 0x04);

  void command(uint8_t c);
  void data(uint8_t c);

  void setContrast(uint8_t val);
  uint8_t getContrast(void);

  uint8_t getBias(void);
  void setBias(uint8_t val);

  void clearDisplay(void);
  void display();
  void updateBoundingBox(uint8_t xmin, uint8_t ymin, uint8_t xmax,
                         uint8_t ymax);

  void setReinitInterval(uint8_t val);
  uint8_t getReinitInterval(void);

  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void setPixel(int16_t x, int16_t y, bool color, uint8_t *buffer);
  bool getPixel(int16_t x, int16_t y, uint8_t *buffer);

  void initDisplay();
  void invertDisplay(bool i);
  void scroll(int8_t vpixels, int8_t hpixels);

private:
  Adafruit_SPIDevice *spi_dev = NULL;
  int8_t _rstpin = -1, _dcpin = -1;

  uint8_t _contrast;        ///< Contrast level, Vop
  uint8_t _bias;            ///< Bias value
  uint8_t _reinit_interval; ///< Reinitialize the display after this many calls
                            ///< to display()
  uint8_t _display_count;   ///< Count for reinit interval

  uint8_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
};

#endif
