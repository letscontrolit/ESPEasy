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

#ifndef SH1106Brzo_h
#define SH1106Brzo_h

#include "OLEDDisplay.h"
#include <brzo_i2c.h>

#if F_CPU == 160000000L
  #define BRZO_I2C_SPEED 1000
#else
  #define BRZO_I2C_SPEED 800
#endif

class SH1106Brzo : public OLEDDisplay {
  private:
      uint8_t             _address;
      uint8_t             _sda;
      uint8_t             _scl;

  public:
    SH1106Brzo(uint8_t _address, uint8_t _sda, uint8_t _scl) {
      this->_address = _address;
      this->_sda = _sda;
      this->_scl = _scl;
    }

    bool connect(){
      brzo_i2c_setup(_sda, _scl, 0);
      return true;
    }

    void display(void) {
    #ifdef OLEDDISPLAY_DOUBLE_BUFFER
       uint8_t minBoundX, minBoundY, maxBoundX, maxBoundY;
       if (!getChangedBoundingBox(minBoundX, minBoundY, maxBoundX, maxBoundY))
         return;

       uint8_t k = 0;
       uint8_t sendBuffer[17];
       sendBuffer[0] = 0x40;

       // Calculate the colum offset 
       uint8_t minBoundXp2H = (minBoundX + 2) & 0x0F;
       uint8_t minBoundXp2L = 0x10 | ((minBoundX + 2) >> 4 );

       brzo_i2c_start_transaction(this->_address, BRZO_I2C_SPEED);

       for (uint8_t y = minBoundY; y <= maxBoundY; y++) {
         sendCommand(0xB0 + y);
         sendCommand(minBoundXp2H);
         sendCommand(minBoundXp2L);
         for (uint8_t x = minBoundX; x <= maxBoundX; x++) {
             k++;
             sendBuffer[k] = buffer[x + y * DISPLAY_WIDTH];
             if (k == 16)  {
               brzo_i2c_write(sendBuffer, 17, true);
               k = 0;
             }
         }
         if (k != 0) {
           brzo_i2c_write(sendBuffer, k + 1, true);
           k = 0;
         }
         yield();
       }
       if (k != 0) {
         brzo_i2c_write(sendBuffer, k + 1, true);
       }
       brzo_i2c_end_transaction();
     #else
     #endif
    }

  private:
    inline void sendCommand(uint8_t com) __attribute__((always_inline)){
      uint8_t command[2] = {0x80 /* command mode */, com};
      brzo_i2c_start_transaction(_address, BRZO_I2C_SPEED);
      brzo_i2c_write(command, 2, true);
      brzo_i2c_end_transaction();
    }
};

#endif
