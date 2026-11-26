// Adafruit_ST7796S.h
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

#ifndef _ADAFRUIT_ST7796S_H_
#define _ADAFRUIT_ST7796S_H_

#include "Adafruit_ST77xx.h"

#define ST7796S_TFTWIDTH 320
#define ST7796S_TFTHEIGHT 480

// ST7796S specific commands
#define ST7796S_RGBCTRL 0xB1
#define ST7796S_FRMCTR1 0xB2
#define ST7796S_PWCTRL1 0xC0
#define ST7796S_PWCTRL2 0xC1
#define ST7796S_VMCTRL1 0xC5
#define ST7796S_VMCTRL2 0xC7
#define ST7796S_PGAMCTRL 0xE0
#define ST7796S_NGAMCTRL 0xE1

/**
 * @brief Enum for ST7796S color order.
 */
enum ST7796S_ColorOrder {
  ST7796S_RGB = 0x00, ///< Red-Green-Blue color order
  ST7796S_BGR = 0x08  ///< Blue-Green-Red color order
};

/**
 * @brief Adafruit driver for the ST7796S TFT display.
 */
class Adafruit_ST7796S : public Adafruit_ST77xx {
public:
  Adafruit_ST7796S(int16_t CS, int16_t RS, int16_t MOSI, int16_t SCLK,
                   int16_t RST = -1);
  Adafruit_ST7796S(int16_t CS, int16_t RS, int16_t RST = -1);
#if !defined(ESP8266)
  Adafruit_ST7796S(SPIClass *spiClass, int16_t CS, int16_t RS, int16_t RST);
#endif

  void init(uint16_t width = ST7796S_TFTWIDTH,
            uint16_t height = ST7796S_TFTHEIGHT, uint8_t rowOffset = 0,
            uint8_t colOffset = 0, ST7796S_ColorOrder colorOrder = ST7796S_RGB);

  void setRotation(uint8_t r);

private:
  ST7796S_ColorOrder _colorOrder;     ///< Color order setting.
  uint16_t windowWidth, windowHeight; ///< Dimensions of the display window.
};

#endif
