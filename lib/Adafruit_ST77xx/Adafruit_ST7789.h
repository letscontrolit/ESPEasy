#ifndef _ADAFRUIT_ST7789H_
#define _ADAFRUIT_ST7789H_

#include "Adafruit_ST77xx.h"

/**
 * 2024-03-09 tonhuisman: Add additional initialization sequences for ST7789 displays, with the intention to get 'm working
 *                        on some devices that seem to use peculiarly configured hardware like LiliGO TTGO T-Display (16MB flash),
 *                        and possibly the T-Display S3
 *                        By default only enabled on ESP32, unless -D ST7789_EXTRA_INIT=1 is defined, f.e. via the build script
 */

#ifndef ST7789_EXTRA_INIT // Enable setting from 'outside', like Platformio.ini
# ifdef ESP8266
#  define ST7789_EXTRA_INIT 0
# endif // ifdef ESP8266
# ifdef ESP32
#  define ST7789_EXTRA_INIT 1
# endif // ifdef ESP32
#endif

/// Subclass of ST77XX type display for ST7789 TFT Driver
class Adafruit_ST7789 : public Adafruit_ST77xx {
public:
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                  int8_t rst = -1);
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst);
#if !defined(ESP8266)
  Adafruit_ST7789(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst);
#endif // end !ESP8266

  void setRotation(uint8_t m);
  void init(uint16_t width, uint16_t height, uint8_t spiMode = SPI_MODE0, uint8_t init_seq = 0u);

protected:
  uint8_t _colstart2 = 0, ///< Offset from the right
      _rowstart2 = 0;     ///< Offset from the bottom

private:
  uint16_t windowWidth;
  uint16_t windowHeight;
  uint8_t _init_seq = 0u;
};

#endif // _ADAFRUIT_ST7789H_
