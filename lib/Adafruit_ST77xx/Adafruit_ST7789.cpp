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

#if ST7789_EXTRA_INIT
static const uint8_t PROGMEM        // Source: https://github.com/Xinyuan-LilyGO/TTGO-T-Display
  alt1_st7789[] =  {                // Init commands for 7789 screens Alternative 1
    21,                             //  21 commands in list:
    ST77XX_SLPOUT, ST_CMD_DELAY,    //  1: Out of sleep mode, no args, w/delay
      120,                          //      120 ms delay
    ST77XX_NORON, ST_CMD_DELAY,     //  2: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_MADCTL, 1,               //  3: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    0xB6, 2,                        //  4: ?JXL240 datasheet?
      0x0A, 0x82,
    ST77XX_COLMOD, 1+ ST_CMD_DELAY, //  5: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_PORCTRL, 5,              //  6: Porch control, Framerate setting
      0x0c, 0x0c, 0x00, 0x33, 0x33,
    ST77XX_GCTRL, 1,                //  7: Gate control, Voltages VGH/VGL
      0x35,
    ST77XX_VCOMS, 1,                //  8: Power settings
      0x28,
    ST77XX_LCMCTRL, 1,              //  9: LCM Control
      0x0C,
    ST77XX_VDVVRHEN, 2,             // 10: VDV & VRH command enable
      0x01, 0xFF,
    ST77XX_VRHS, 1,                 // 11: VRH set
      0x10,
    ST77XX_VDVSET, 1,               // 12: VDV set
      0x20,
    ST77XX_FRCTR2, 1,               // 13: FR Control 2
      0x0F,
    ST77XX_PWCTRL1, 2,              // 14: Power Control 1
      0xA4, 0xA1,
    ST77XX_PVGAMCTRL, 14,           // 15: Positive Voltage Gamma control
      0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17,
    ST77XX_NVGAMCTRL, 14,           // 16: Negative Voltage Gamma control
      0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E,
    ST77XX_INVON, ST_CMD_DELAY,     // 17: hack
      10,
    ST77XX_CASET  , 4,              // 18: Column addr set, 4 args, no delay:
      0x00,
      0,              //     XSTART = 0
      0,
      240,            //     XEND = 240
    ST77XX_RASET  , 4,              // 19: Row addr set, 4 args, no delay:
      0x00,
      0,              //     YSTART = 0
      320>>8,
      320&0xFF,       //     YEND = 320
    ST77XX_INVON, ST_CMD_DELAY,     // 20: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON, ST_CMD_DELAY,    // 21: Main screen turn on, no args, delay
      255                           //     120 ms delay
  };

static const uint8_t PROGMEM        // Source: https://github.com/Bodmer/TFT_eSPI (ST7789_init.h, _NOT_ INIT_SEQUENCE_3)
  alt2_st7789[] =  {                // Init commands for 7789 screens Alternative 2
    21,                             //  21 commands in list:
    ST77XX_SLPOUT, ST_CMD_DELAY,    //  1: Out of sleep mode, no args, w/delay
      120,                          //      120 ms delay
    ST77XX_NORON, ST_CMD_DELAY,     //  2: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_MADCTL, 1,               //  3: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    0xB6, 2,                        //  4: ?JXL240 datasheet?
      0x0A, 0x82,
    ST77XX_RAMCTRL, 2,              //  5: RAM control
      0x00, 0xE0,                   //     5 to 6-bit conversion: r0 = r5, b0 = b5
    ST77XX_COLMOD, 1+ ST_CMD_DELAY, //  6: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_PORCTRL, 5,              //  7: Porch control, Framerate setting
      0x0c, 0x0c, 0x00, 0x33, 0x33,
    ST77XX_GCTRL, 1,                //  8: Gate control, Voltages VGH/VGL
      0x35,
    ST77XX_VCOMS, 1,                //  9: Power settings
      0x28,
    ST77XX_LCMCTRL, 1,              // 10: LCM Control
      0x0C,
    ST77XX_VDVVRHEN, 2,             // 11: VDV & VRH command enable
      0x01, 0xFF,
    ST77XX_VRHS, 1,                 // 12: VRH set
      0x10,
    ST77XX_VDVSET, 1,               // 13: VDV set
      0x20,
    ST77XX_FRCTR2, 1,               // 14: FR Control 2
      0x0F,
    ST77XX_PWCTRL1, 2,              // 15: Power Control 1
      0xA4, 0xA1,
    ST77XX_PVGAMCTRL, 14,           // 16: Positive Voltage Gamma control
      0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17,
    ST77XX_NVGAMCTRL, 14,           // 17: Negative Voltage Gamma control
      0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E,
    ST77XX_INVON, ST_CMD_DELAY,     // 18: hack
      10,
    ST77XX_CASET  , 4,              // 19: Column addr set, 4 args, no delay:
      0x00,
      0,              //     XSTART = 0
      0,
      239,            //     XEND = 239
    ST77XX_RASET  , 4,              // 20: Row addr set, 4 args, no delay:
      0x00,
      0,              //     YSTART = 0
      319>>8,
      319&0xFF,       //     YEND = 319
    ST77XX_DISPON, ST_CMD_DELAY,    // 21: Main screen turn on, no args, delay
      120                           //     120 ms delay
  };

static const uint8_t PROGMEM        // Source: https://github.com/Bodmer/TFT_eSPI (ST7789_init.h, _WITH_ INIT_SEQUENCE_3)
  alt3_st7789[] =  {                // Init commands for 7789 screens Alternative 2
    18,                             //  18 commands in list:
    ST77XX_SLPOUT, ST_CMD_DELAY,    //  1: Out of sleep mode, no args, w/delay
      120,                          //      120 ms delay
    ST77XX_NORON, ST_CMD_DELAY,     //  2: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_MADCTL, 1,               //  3: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    0xB6, 2,                        //  4: ?JXL240 datasheet?
      0x0A, 0x82,
    ST77XX_COLMOD, 1+ ST_CMD_DELAY, //  5: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_PORCTRL, 5,              //  6: Porch control, Framerate setting
      0x0c, 0x0c, 0x00, 0x33, 0x33,
    ST77XX_GCTRL, 1,                //  7: Gate control, Voltages VGH/VGL
      0x75,
    ST77XX_VCOMS, 1,                //  8: Power settings
      0x28,
    ST77XX_LCMCTRL, 1,              //  9: LCM Control
      0x2C,
    ST77XX_VDVVRHEN, 1,             // 10: VDV & VRH command enable
      0x01,
    ST77XX_VRHS, 1,                 // 11: VRH set
      0x1F,
    ST77XX_FRCTR2, 1,               // 12: FR Control 2
      0x13,
    ST77XX_PWCTRL1, 1,              // 13: Power Control 1
      0xA7,
    ST77XX_PWCTRL1, 2,              // 14: Power Control 1
      0xA4, 0xA1,
    0xD6, 1,                        // 15: ?
      0xA1,
    ST77XX_PVGAMCTRL, 14,           // 16: Positive Voltage Gamma control
      0xF0, 0x05, 0x0A, 0x06, 0x06, 0x03, 0x2B, 0x32, 0x43, 0x36, 0x11, 0x10, 0x2B, 0x32,
    ST77XX_NVGAMCTRL, 14,           // 17: Negative Voltage Gamma control
      0xF0, 0x08, 0x0C, 0x0B, 0x09, 0x24, 0x2B, 0x22, 0x43, 0x38, 0x15, 0x16, 0x2F, 0x37,
    // ST77XX_CASET  , 4,              // 18: Column addr set, 4 args, no delay:
    //   0x00,
    //   0,              //     XSTART = 0
    //   0,
    //   239,            //     XEND = 239
    // ST77XX_RASET  , 4,              // 19: Row addr set, 4 args, no delay:
    //   0x00,
    //   0,              //     YSTART = 0
    //   319>>8,
    //   319&0xFF,       //     YEND = 319
    ST77XX_DISPON, ST_CMD_DELAY,    // 18: Main screen turn on, no args, delay
      120                           //     120 ms delay
  };
#endif // if ST7789_EXTRA_INIT
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
void Adafruit_ST7789::init(uint16_t width, uint16_t height, uint8_t mode, uint8_t init_seq) {
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

  _init_seq = init_seq;

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

  const uint8_t *init_ = generic_st7789;

  #if ST7789_EXTRA_INIT
  if (1 == _init_seq) {
    init_ = alt1_st7789;
  } else if (2 == _init_seq) {
    init_ = alt2_st7789;
  } else if (3 == _init_seq) {
    init_ = alt3_st7789;
  }
  #endif // if ST7789_EXTRA_INIT
  displayInit(init_);
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
    #if ST7789_EXTRA_INIT
    if (_init_seq > 0) {
      madctl    = ST77XX_MADCTL_BGR;
      _colstart = 52;
      _rowstart = 40;
    } else
    #endif // if ST7789_EXTRA_INIT
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 1:
    #if ST7789_EXTRA_INIT
    if (_init_seq > 0) {
      madctl    = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_BGR;
      _colstart = 40;
      _rowstart = 53;
    } else
    #endif // if ST7789_EXTRA_INIT
    {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    }
    _xstart = _rowstart;
    _ystart = _colstart;
    _height = windowWidth;
    _width = windowHeight;
    break;
  case 2:
    #if ST7789_EXTRA_INIT
    if (_init_seq > 0) {
      madctl     = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
      _colstart2 = 53;
      _rowstart2 = 40;
    } else
    #endif // if ST7789_EXTRA_INIT
    {
      madctl = ST77XX_MADCTL_RGB;
    }
    _xstart = _colstart2;
    _ystart = _rowstart2;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 3:
    #if ST7789_EXTRA_INIT
    if (_init_seq > 0) {
      madctl     = ST77XX_MADCTL_MV | ST77XX_MADCTL_MY | ST77XX_MADCTL_BGR;
      _colstart2 = 40;
      _rowstart2 = 52;
    } else
    #endif // if ST7789_EXTRA_INIT
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    }
    _xstart = _rowstart2;
    _ystart = _colstart2;
    _height = windowWidth;
    _width = windowHeight;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
