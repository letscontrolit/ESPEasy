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

#ifndef SSD1306Wire_h
#define SSD1306Wire_h

#include "OLEDDisplay.h"
#include <Wire.h>

class SSD1306Wire : public OLEDDisplay {
  private:
      uint8_t             _address;
      uint8_t             _sda;
      uint8_t             _scl;

  public:
    SSD1306Wire(uint8_t _address, uint8_t _sda, uint8_t _scl, int width = DISPLAY_WIDTH, int height = DISPLAY_HEIGHT)
    : OLEDDisplay(width, height) {
      this->_address = _address;
      this->_sda = _sda;
      this->_scl = _scl;
    }

    bool connect() {
      Wire.begin(this->_sda, this->_scl);
      return true;
    }

    void display(void) {
      const int x_offset = (128 - this->width()) / 2;
      #ifdef OLEDDISPLAY_DOUBLE_BUFFER
        uint8_t minBoundX, minBoundY, maxBoundX, maxBoundY;
        if (!getChangedBoundingBox(minBoundX, minBoundY, maxBoundX, maxBoundY))
          return;

        sendCommand(COLUMNADDR);
        sendCommand(x_offset + minBoundX);
        sendCommand(x_offset + maxBoundX);

        sendCommand(PAGEADDR);
        sendCommand(minBoundY);
        sendCommand(maxBoundY);

        uint8_t k = 0;
        for (uint8_t y = minBoundY; y <= maxBoundY; y++) {
          for (uint8_t x = minBoundX; x <= maxBoundX; x++) {
            if (k == 0) {
              Wire.beginTransmission(_address);
              Wire.write(0x40);
            }
            Wire.write(buffer[x + y * this->width()]);
            k++;
            if (k == 16)  {
              Wire.endTransmission();
              k = 0;
            }
          }
          yield();
        }

        if (k != 0) {
          Wire.endTransmission();
        }
      #else

        sendCommand(COLUMNADDR);
        sendCommand(x_offset);
        sendCommand(x_offset + (this->width() - 1));

        sendCommand(PAGEADDR);
        sendCommand(0x0);
        sendCommand((this->height() / 8) - 1);

        for (uint16_t i=0; i < DISPLAY_BUFFER_SIZE; i++) {
          Wire.beginTransmission(this->_address);
          Wire.write(0x40);
          for (uint8_t x = 0; x < 16; x++) {
            Wire.write(buffer[i]);
            i++;
          }
          i--;
          Wire.endTransmission();
        }
      #endif
    }

  private:
    inline void sendCommand(uint8_t command) __attribute__((always_inline)){
      Wire.beginTransmission(_address);
      Wire.write(0x80);
      Wire.write(command);
      Wire.endTransmission();
    }


};

#endif
