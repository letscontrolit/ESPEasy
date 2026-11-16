// Adafruit_ST7796S.cpp
//
// MIT License
//
// Copyright (c) 2025 Limor Fried/Ladyada for Adafruit Industries
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Adafruit_ST7796S.h"

static const uint8_t PROGMEM st7796s_init[] = {14, // 14 commands
                                               ST77XX_SWRESET,
                                               ST_CMD_DELAY, // Software reset
                                               150,
                                               0xF0,
                                               1, // Unlock manufacturer
                                               0xC3,
                                               0xF0,
                                               1,
                                               0x96,
                                               0xC5,
                                               1, // VCOM Control
                                               0x1C,
                                               ST77XX_MADCTL,
                                               1, // Memory Access
                                               0x48,
                                               ST77XX_COLMOD,
                                               1, // Color Mode - 16 bit
                                               0x55,
                                               0xB0,
                                               1, // Interface Control
                                               0x80,
                                               0xB4,
                                               1, // Inversion Control
                                               0x00,
                                               0xB6,
                                               3, // Display Function Control
                                               0x80,
                                               0x02,
                                               0x3B,
                                               0xB7,
                                               1, // Entry Mode
                                               0xC6,
                                               0xF0,
                                               1, // Lock manufacturer commands
                                               0x69,
                                               0xF0,
                                               1,
                                               0x3C,
                                               ST77XX_SLPOUT,
                                               ST_CMD_DELAY, // Exit sleep
                                               150,
                                               ST77XX_DISPON,
                                               ST_CMD_DELAY, // Display on
                                               150};

/**
 * @brief Constructor with software SPI.
 * @param CS Chip select pin.
 * @param RS Data/command pin.
 * @param MOSI SPI MOSI pin.
 * @param SCLK SPI clock pin.
 * @param RST Reset pin (optional).
 */
Adafruit_ST7796S::Adafruit_ST7796S(int16_t CS, int16_t RS, int16_t MOSI,
                                   int16_t SCLK, int16_t RST)
    : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, CS, RS, MOSI, SCLK,
                      RST) {}

/**
 * @brief Constructor with hardware SPI.
 * @param CS Chip select pin.
 * @param RS Data/command pin.
 * @param RST Reset pin (optional).
 */
Adafruit_ST7796S::Adafruit_ST7796S(int16_t CS, int16_t RS, int16_t RST)
    : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, CS, RS, RST) {}

#if !defined(ESP8266)
/**
 * @brief Constructor with hardware SPI and custom SPI class.
 * @param spiClass Pointer to SPI class.
 * @param CS Chip select pin.
 * @param RS Data/command pin.
 * @param RST Reset pin (optional).
 */
Adafruit_ST7796S::Adafruit_ST7796S(SPIClass *spiClass, int16_t CS, int16_t RS,
                                   int16_t RST)
    : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, spiClass, CS, RS,
                      RST) {}
#endif

/**
 * @brief Initialize the display.
 * @param width Display width in pixels.
 * @param height Display height in pixels.
 * @param rowOffset Row offset for display.
 * @param colOffset Column offset for display.
 * @param colorOrder Color order (RGB or BGR).
 */
void Adafruit_ST7796S::init(uint16_t width, uint16_t height, uint8_t rowOffset,
                            uint8_t colOffset, ST7796S_ColorOrder colorOrder) {
  _width = width;
  _height = height;
  _rowstart = rowOffset;
  _colstart = colOffset;
  _colorOrder = colorOrder;
  windowWidth = width;
  windowHeight = height;

  commonInit(NULL);
  displayInit(st7796s_init);
  invertOnCommand = ST77XX_INVOFF;
  invertOffCommand = ST77XX_INVON;
  invertDisplay(false);
  setRotation(0);
}

/**
 * @brief Set the display rotation.
 * @param m Rotation value (0-3).
 */
void Adafruit_ST7796S::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_RGB | _colorOrder;
    _xstart = _colstart;
    _ystart = _rowstart;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB | _colorOrder;
    _xstart = _rowstart;
    _ystart = _colstart;
    _height = windowWidth;
    _width = windowHeight;
    break;
  case 2:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB | _colorOrder;
    _xstart = _colstart;
    _ystart = _rowstart;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MX | ST77XX_MADCTL_MV |
             ST77XX_MADCTL_RGB | _colorOrder;
    _xstart = _rowstart;
    _ystart = _colstart;
    _height = windowWidth;
    _width = windowHeight;
    break;
  }

  // Serial.println(madctl, HEX);
  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
