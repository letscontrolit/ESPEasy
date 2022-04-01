#include "Adafruit_ST7789.h"
#include "Adafruit_ST77xx.h"

// CONSTRUCTORS ************************************************************

/*!
    @brief  Instantiate Adafruit ST7789 driver with software SPI
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  mosi  SPI MOSI pin #
    @param  sclk  SPI Clock pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7789::Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                                 int8_t rst)
    : Adafruit_ST77xx(240, 320, cs, dc, mosi, sclk, rst) {}

/*!
    @brief  Instantiate Adafruit ST7789 driver with hardware SPI
    @param  cs   Chip select pin #
    @param  dc   Data/Command pin #
    @param  rst  Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7789::Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST77xx(240, 320, cs, dc, rst) {}

#if !defined(ESP8266)
/*!
    @brief  Instantiate Adafruit ST7789 driver with selectable hardware SPI
    @param  spiClass  Pointer to an SPI device to use (e.g. &SPI1)
    @param  cs        Chip select pin #
    @param  dc        Data/Command pin #
    @param  rst       Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7789::Adafruit_ST7789(SPIClass *spiClass, int8_t cs, int8_t dc,
                                 int8_t rst)
    : Adafruit_ST77xx(240, 320, spiClass, cs, dc, rst) {}
#endif // end !ESP8266

// SCREEN INITIALIZATION ***************************************************

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.

// clang-format off

static const uint8_t PROGMEM
  generic_st7789[] =  {                // Init commands for 7789 screens
    9,                              //  9 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      150,                          //     ~150 ms delay
    ST77XX_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      10,                          //      10 ms delay
    ST77XX_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST77XX_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
      0x00,
      0,        //     XSTART = 0
      0,
      240,  //     XEND = 240
    ST77XX_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
      0x00,
      0,             //     YSTART = 0
      320>>8,
      320&0xFF,  //     YEND = 320
    ST77XX_INVON  ,   ST_CMD_DELAY,  //  7: hack
      10,
    ST77XX_NORON  ,   ST_CMD_DELAY, //  8: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON ,   ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
      10 };                          //    10 ms delay

// clang-format on

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7789 displays
    @param  width  Display width
    @param  height Display height
    @param  mode   SPI data mode; one of SPI_MODE0, SPI_MODE1, SPI_MODE2
                   or SPI_MODE3 (do NOT pass the numbers 0,1,2 or 3 -- use
                   the defines only, the values are NOT the same!)
*/
/**************************************************************************/
void Adafruit_ST7789::init(uint16_t width, uint16_t height, uint8_t mode) {
  // Save SPI data mode. commonInit() calls begin() (in Adafruit_ST77xx.cpp),
  // which in turn calls initSPI() (in Adafruit_SPITFT.cpp), passing it the
  // value of spiMode. It's done this way because begin() really should not
  // be modified at this point to accept an SPI mode -- it's a virtual
  // function required in every Adafruit_SPITFT subclass and would require
  // updating EVERY such library...whereas, at the moment, we know that
  // certain ST7789 displays are the only thing that may need a non-default
  // SPI mode, hence this roundabout approach...
  spiMode = mode;
  // (Might get added similarly to other display types as needed on a
  // case-by-case basis.)

  commonInit(NULL);

  if (width < 240) {
    // 1.14" display
    _rowstart = _rowstart2 = (int)((320 - height + 1) / 2);
    _colstart = (int)((240 - width + 1) / 2);
    _colstart2 = (int)((240 - width) / 2);
  } else if (width == 240 && height == 280) {
    // 1.69" display
    _rowstart = 20;
    _rowstart2 = 0;
    _colstart = _colstart2 = 0;
  } else {
    // 1.3", 1.54", and 2.0" displays
    _rowstart = (320 - height);
    _rowstart2 = 0;
    _colstart = _colstart2 = (240 - width);
  }

  windowWidth = width;
  windowHeight = height;

  displayInit(generic_st7789);
  setRotation(0);
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST7789::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _xstart = _rowstart;
    _ystart = _colstart;
    _height = windowWidth;
    _width = windowHeight;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    _xstart = _colstart2;
    _ystart = _rowstart2;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _xstart = _rowstart2;
    _ystart = _colstart2;
    _height = windowWidth;
    _width = windowHeight;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
