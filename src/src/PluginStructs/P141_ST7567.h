#ifndef PLUGINSTRUCTS_P141_ST7567_H
#define PLUGINSTRUCTS_P141_ST7567_H

#include "../../_Plugin_Helper.h"
#if defined(USES_P141) && FEATURE_ST7567

# include <Adafruit_GFX.h>
# include <Adafruit_SPIDevice.h>

/****************************************************************************
 * Minimal Adafruit_GFX driver for ST7567 based 128x64 monochrome SPI LCDs,
 * like the OPEN-SMART 1.8" LCD module.
 * Init sequence based on the u8g2 st7567_os12864 driver (https://github.com/olikraus/u8g2)
 ***************************************************************************/

# define ST7567_LCDWIDTH        128
# define ST7567_LCDHEIGHT       64
# define ST7567_X_OFFSET        4   // Controller has 132 columns, visible area starts at column 4 (ADC reversed)
# define ST7567_CONTRAST_MAX    63  // Electronic volume range 0..63

class P141_ST7567 : public Adafruit_GFX {
public:

  P141_ST7567(int8_t    dc_pin,
              int8_t    cs_pin,
              int8_t    rst_pin,
              SPIClass *theSPI = &SPI);
  virtual ~P141_ST7567();

  bool begin(uint8_t contrast = 12);
  void display();
  void clearDisplay();
  void setContrast(uint8_t val); // 0..63
  void drawPixel(int16_t  x,
                 int16_t  y,
                 uint16_t color) override;
  void invertDisplay(bool i) override;

private:

  void command(uint8_t c);

  Adafruit_SPIDevice *_spi_dev = nullptr;
  int8_t              _dcpin   = -1;
  int8_t              _rstpin  = -1;
  uint8_t             _buffer[ST7567_LCDWIDTH * ST7567_LCDHEIGHT / 8]{};
};

#endif // if defined(USES_P141) && FEATURE_ST7567
#endif // ifndef PLUGINSTRUCTS_P141_ST7567_H
