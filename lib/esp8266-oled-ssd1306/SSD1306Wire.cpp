#include "SSD1306Wire.h"

    SSD1306Wire::SSD1306Wire(uint8_t address, uint8_t sda, uint8_t scl, int width, int height)
    : OLEDDisplay(width, height) {
      this->_address = address;
      this->_sda = sda;
      this->_scl = scl;
    }

    bool SSD1306Wire::connect()  {
      // Wire.begin(this->_sda, this->_scl); // ESPEasy already has Wire initialized properly
      return true;
    }

    void SSD1306Wire::display(void) {
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

    void SSD1306Wire::sendCommand(uint8_t command) {
      Wire.beginTransmission(_address);
      Wire.write(0x80);
      Wire.write(command);
      Wire.endTransmission();
    }
