#include "SH1106Wire.h"



    SH1106Wire::SH1106Wire(uint8_t address, uint8_t sda, uint8_t scl) :
      _address(address),
      _sda(sda),
      _scl(scl) {}

    bool SH1106Wire::connect() {
      // Wire.begin(this->_sda, this->_scl); // ESPEasy already has Wire initialized properly
      return true;
    }

    void SH1106Wire::display(void) {
      #ifdef OLEDDISPLAY_DOUBLE_BUFFER
        uint8_t minBoundX, minBoundY, maxBoundX, maxBoundY;
        if (!getChangedBoundingBox(minBoundX, minBoundY, maxBoundX, maxBoundY))
          return;


        // Calculate the colum offset
        uint8_t minBoundXp2H = (minBoundX + 2) & 0x0F;
        uint8_t minBoundXp2L = 0x10 | ((minBoundX + 2) >> 4 );

        uint8_t k = 0;
        for (uint8_t y = minBoundY; y <= maxBoundY; y++) {
          sendCommand(0xB0 + y);
          sendCommand(minBoundXp2H);
          sendCommand(minBoundXp2L);
          for (uint8_t x = minBoundX; x <= maxBoundX; x++) {
            if (k == 0) {
              Wire.beginTransmission(_address);
              Wire.write(0x40);
            }
            Wire.write(buffer[x + y * DISPLAY_WIDTH]);
            k++;
            if (k == 16)  {
              Wire.endTransmission();
              k = 0;
            }
          }
          if (k != 0)  {
            Wire.endTransmission();
            k = 0;
          }
          yield();
        }

        if (k != 0) {
          Wire.endTransmission();
        }
      #else
        uint8_t * p = &buffer[0];
        for (uint8_t y=0; y<8; y++) {
          sendCommand(0xB0+y);
          sendCommand(0x02);
          sendCommand(0x10);
          for( uint8_t x=0; x<8; x++) {
            Wire.beginTransmission(_address);
            Wire.write(0x40);
            for (uint8_t k = 0; k < 16; k++) {
              Wire.write(*p++);
            }
            Wire.endTransmission();
          }
        }
      #endif
    }

    void SH1106Wire::sendCommand(uint8_t command) {
      Wire.beginTransmission(_address);
      Wire.write(0x80);
      Wire.write(command);
      Wire.endTransmission();
    }
