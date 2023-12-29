/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
 */

#ifndef SSD1306Spi_h
#define SSD1306Spi_h

#include "OLEDDisplay.h"
#include <SPI.h>

#if F_CPU == 160000000L
  #define BRZO_I2C_SPEED 1000
#else
  #define BRZO_I2C_SPEED 800
#endif

class SSD1306Spi : public OLEDDisplay {
  private:
      uint8_t             _rst;
      uint8_t             _dc;
      uint8_t             _cs;

  public:
    SSD1306Spi(uint8_t _rst, uint8_t _dc, uint8_t _cs) {
      this->_rst = _rst;
      this->_dc  = _dc;
      this->_cs  = _cs;
    }

    bool connect(){
      pinMode(_dc, OUTPUT);
      pinMode(_cs, OUTPUT);
      pinMode(_rst, OUTPUT);

      SPI.begin ();
      SPI.setClockDivider (SPI_CLOCK_DIV2);

      // Pulse Reset low for 10ms
      digitalWrite(_rst, HIGH);
      delay(1);
      digitalWrite(_rst, LOW);
      delay(10);
      digitalWrite(_rst, HIGH);
      return true;
    }

    void display(void) {
    #ifdef OLEDDISPLAY_DOUBLE_BUFFER
       uint8_t minBoundX, minBoundY, maxBoundX, maxBoundY;
       if (!getChangedBoundingBox(minBoundX, minBoundY, maxBoundX, maxBoundY))
         return;

       sendCommand(COLUMNADDR);
       sendCommand(minBoundX);
       sendCommand(maxBoundX);

       sendCommand(PAGEADDR);
       sendCommand(minBoundY);
       sendCommand(maxBoundY);

       digitalWrite(_cs, HIGH);
       digitalWrite(_dc, HIGH);   // data mode
       digitalWrite(_cs, LOW);
       for (uint8_t y = minBoundY; y <= maxBoundY; y++) {
         for (uint8_t x = minBoundX; x <= maxBoundX; x++) {
           SPI.transfer(buffer[x + y * DISPLAY_WIDTH]);
         }
         yield();
       }
       digitalWrite(_cs, HIGH);
     #else
       // No double buffering
       sendCommand(COLUMNADDR);
       sendCommand(0x0);
       sendCommand(0x7F);

       sendCommand(PAGEADDR);
       sendCommand(0x0);
       sendCommand(0x7);

        digitalWrite(_cs, HIGH);
        digitalWrite(_dc, HIGH);   // data mode
        digitalWrite(_cs, LOW);
        for (uint16_t i=0; i<DISPLAY_BUFFER_SIZE; i++) {
          SPI.transfer(buffer[i]);
          yield();
        }
        digitalWrite(_cs, HIGH);
     #endif
    }

  private:
    inline void sendCommand(uint8_t com) __attribute__((always_inline)){
      digitalWrite(_cs, HIGH);
      digitalWrite(_dc, LOW);
      digitalWrite(_cs, LOW);
      SPI.transfer(com);
      digitalWrite(_cs, HIGH);
    }
};

#endif
