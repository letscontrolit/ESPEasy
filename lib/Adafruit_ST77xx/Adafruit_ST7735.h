#ifndef _ADAFRUIT_ST7735H_
#define _ADAFRUIT_ST7735H_

#include "Adafruit_ST77xx.h"

// some flags for initR() :(
#define INITR_GREENTAB 0x00
#define INITR_REDTAB 0x01
#define INITR_BLACKTAB 0x02
#define INITR_18GREENTAB INITR_GREENTAB
#define INITR_18REDTAB INITR_REDTAB
#define INITR_18BLACKTAB INITR_BLACKTAB
#define INITR_144GREENTAB 0x01
#define INITR_MINI160x80 0x04
#define INITR_HALLOWING 0x05

// Some register settings
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Some ready-made 16-bit ('565') color settings:
#define ST7735_BLACK ST77XX_BLACK
#define ST7735_WHITE ST77XX_WHITE
#define ST7735_RED ST77XX_RED
#define ST7735_GREEN ST77XX_GREEN
#define ST7735_BLUE ST77XX_BLUE
#define ST7735_CYAN ST77XX_CYAN
#define ST7735_MAGENTA ST77XX_MAGENTA
#define ST7735_YELLOW ST77XX_YELLOW
#define ST7735_ORANGE ST77XX_ORANGE

/// Subclass of ST77XX for ST7735B and ST7735R TFT Drivers:
class Adafruit_ST7735 : public Adafruit_ST77xx {
public:
  Adafruit_ST7735(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst);
  Adafruit_ST7735(int8_t cs, int8_t dc, int8_t rst);
#if !defined(ESP8266)
  Adafruit_ST7735(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst);
#endif // end !ESP8266

  // Differences between displays (usu. identified by colored tab on
  // plastic overlay) are odd enough that we need to do this 'by hand':
  void initB(void);                             // for ST7735B displays
  void initR(uint8_t options = INITR_GREENTAB); // for ST7735R

  void setRotation(uint8_t m);

private:
  uint8_t tabcolor;
};

#endif // _ADAFRUIT_ST7735H_
