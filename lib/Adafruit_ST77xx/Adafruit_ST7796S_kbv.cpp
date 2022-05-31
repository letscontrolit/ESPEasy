/*!
 * These displays use SPI to communicate, 4 or 5 pins are required
 * to interface (RST is optional).
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
 * Adafruit_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_ST7796S_kbv.h"
#ifndef ARDUINO_STM32_FEATHER
# include "pins_arduino.h"
# ifndef RASPI
#  include "wiring_private.h"
# endif // ifndef RASPI
#endif  // ifndef ARDUINO_STM32_FEATHER
#include <limits.h>

#if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)
# define SPI_DEFAULT_FREQ 16000000

// Teensy 3.0, 3.1/3.2, 3.5, 3.6
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || \
  defined(__MK64FX512__) || defined(__MK66FX1M0__)
# define SPI_DEFAULT_FREQ 40000000
#elif defined(__AVR__) || defined(TEENSYDUINO)
# define SPI_DEFAULT_FREQ 8000000
#elif defined(ESP8266) || defined(ESP32)
# define SPI_DEFAULT_FREQ 40000000
#elif defined(RASPI)
# define SPI_DEFAULT_FREQ 80000000
#elif defined(ARDUINO_ARCH_STM32F1)
# define SPI_DEFAULT_FREQ 36000000
#else // if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)
# define SPI_DEFAULT_FREQ 24000000 ///< Default SPI data clock frequency
#endif // if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ST7796S driver with software SPI
    @param    cs    Chip select pin #
    @param    dc    Data/Command pin #
    @param    mosi  SPI MOSI pin #
    @param    sclk  SPI Clock pin #
    @param    rst   Reset pin # (optional, pass -1 if unused)
    @param    miso  SPI MISO pin # (optional, pass -1 if unused)
 */

/**************************************************************************/
Adafruit_ST7796S_kbv::Adafruit_ST7796S_kbv(int8_t cs, int8_t dc, int8_t mosi,
                                           int8_t sclk, int8_t rst, int8_t miso)
  : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, cs, dc, mosi, sclk,
                    rst, miso) {}

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ST7796S driver with hardware SPI using the
            default SPI peripheral.
    @param  cs   Chip select pin # (OK to pass -1 if CS tied to GND).
    @param  dc   Data/Command pin # (required).
    @param  rst  Reset pin # (optional, pass -1 if unused).
 */

/**************************************************************************/
Adafruit_ST7796S_kbv::Adafruit_ST7796S_kbv(int8_t cs, int8_t dc, int8_t rst)
  : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, cs, dc, rst) {}

#if !defined(ESP8266)

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ST7796S driver with hardware SPI using
            a specific SPI peripheral (not necessarily default).
    @param  spiClass  Pointer to SPI peripheral (e.g. &SPI or &SPI1).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and
                      CS is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
 */

/**************************************************************************/
Adafruit_ST7796S_kbv::Adafruit_ST7796S_kbv(SPIClass *spiClass, int8_t dc, int8_t cs,
                                           int8_t rst)
  : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, spiClass, cs, dc,
                    rst) {}

#endif // end !ESP8266

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ST7796S driver using parallel interface.
    @param  busWidth  If tft16 (enumeration in Adafruit_SPITFT.h), is a
                      16-bit interface, else 8-bit.
    @param  d0        Data pin 0 (MUST be a byte- or word-aligned LSB of a
                      PORT register -- pins 1-n are extrapolated from this).
    @param  wr        Write strobe pin # (required).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and CS
                      is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
    @param  rd        Read strobe pin # (optional, pass -1 if unused).
 */

/**************************************************************************/
// Adafruit_ST7796S_kbv::Adafruit_ST7796S_kbv(tftBusWidth busWidth, int8_t d0, int8_t wr,
//                                            int8_t dc, int8_t cs, int8_t rst, int8_t rd)
//   : Adafruit_ST77xx(ST7796S_TFTWIDTH, ST7796S_TFTHEIGHT, busWidth, d0, wr, dc,
//                     cs, rst, rd) {}

// clang-format off
static const uint8_t PROGMEM initcmd[] = {
  //  (COMMAND_BYTE), n, data_bytes....
  0x01,                 0x80,                     // Soft reset, then delay 150 ms
  0xF0,                 1,                  0xC3, // ?? Unlock Manufacturer
  0xF0,                 1,                  0x96,
#if 0
#elif 0                                           // LCDWIKI
  0x36,                 1,                  0x68,
  0x3A,                 1,                  0x05,
  0               0xB0, 1,                  0x80,
  0xB6,                 2,                  0x00, 0x02,
  0xB5,                 4,                  0x02, 0x03, 0x00,  0x04,
  0xB1,                 2,                  0x80, 0x10,
  0xB4,                 1,                  0x00,
  0xB7,                 1,                  0xC6,
  0xC5,                 1,                  0x24,
  0xE4,                 1,                  0x31,
  0xE8,                 8,                  0x40, 0x8A, 0x00,  0x00, 0x29,  0x19,  0xA5, 0x33,
  0xC2,                 1,                  0xA7,
  0xE0,                 14,                 0xF0, 0x09, 0x13,  0x12, 0x12,  0x2B,  0x3C, 0x44,0x4B,  0x1B,  0x18, 0x17, 0x1D, 0x21,
  0xE1,                 14,                 0xF0, 0x09, 0x13,  0x0C, 0x0D,  0x27,  0x3B, 0x44,0x4D,  0x0B,  0x17, 0x17, 0x1D, 0x21,
  0x36,                 1,                  0x48,

#elif 0                                                                                                                             // TFT_eSPI
  0x36,                 1,                  0x48,
  0x3A,                 1,                  0x05,                                                                                   // Interlace
                                                                                                                                    // Pixel
                                                                                                                                    // Format
                                                                                                                                    // [XX]
  0xB4,                 1,                  0x01,                                                                                   // Inversion
                                                                                                                                    // Control
                                                                                                                                    // [01]
  0xB6,                 3,                  0x80, 0x02, 0x3B,                                                                       // Display
                                                                                                                                    // Function
                                                                                                                                    // Control
                                                                                                                                    // [80
                                                                                                                                    // 02
                                                                                                                                    // 3B]
  0xE8,                 8,                  0x40, 0x8A, 0x00,  0x00, 0x29,  0x19,  0xA5, 0x33,                                      // Adjustment
                                                                                                                                    // Control
                                                                                                                                    // 3 [40
                                                                                                                                    // 8A 00
                                                                                                                                    // 00 25
                                                                                                                                    // 0A 38
                                                                                                                                    // 33]
  0xC1,                 1,                  0x06,                                                                                   // Power
                                                                                                                                    // Control
                                                                                                                                    // 2 [13]
  0xC2,                 1,                  0xA7,                                                                                   // Power
                                                                                                                                    // Control
                                                                                                                                    // 3 [A?]
  0xC5,                 1,                  0x18,                                                                                   // VCOM=0.9
                                                                                                                                    // [1C]
  // 0x11, 0x80,                 //delay 150 ms
  (0xE0),               14,                 0xF0, 0x09, 0x0B,  0x06, 0x04,  0x15,  0x2F, 0x54,0x42,  0x3C,  0x17, 0x14, 0x18, 0x1B, // PVGAMCTRL:
                                                                                                                                    // Positive
                                                                                                                                    // Voltage
                                                                                                                                    // Gamma
                                                                                                                                    // control
  (0xE1),               14,                 0xE0, 0x09, 0x0B,  0x06, 0x04,  0x03,  0x2B, 0x43,0x42,  0x3B,  0x16, 0x14, 0x17, 0x1B, // NVGAMCTRL:
                                                                                                                                    // Negative
                                                                                                                                    // Voltage
                                                                                                                                    // Gamma
                                                                                                                                    // control
#else // if 0

  //    0xC0, 2, 0x10, 0x10,        //Power Control 1 [80 25]
  //    0xC1, 1, 0x41,              //Power Control 2 [13]
  0xC5,                 1,                  0x1C,             // VCOM  Control 1 [1C]
  0x36,                 1,                  0x48,             // Memory Access [00]
  0x3A,                 1,                  0x55,             // 565
  0xB0,                 1,                  0x80,             // Interface     [00]
  // 0xB1, 2, 0xB0, 0x11,        //Frame Rate Control [A0 10]
  0xB4,                 1,                  0x01,             // Inversion Control [01]
  0xB6,                 3,                  0x80, 0x02, 0x3B, // Display Function Control [80 02 3B] .kbv SS=1, NL=480
  0xB7,                 1,                  0xC6,             // Entry Mode      [06]
  //    0x3A, 1, 0x66,              //Interlace Pixel Format [XX]
  //    0xF7, 4, 0xA9, 0x51, 0x2C, 0x82,    //Adjustment Control 3 [A9 51 2C 82]
#endif // if 0
  0xF0,                 1,                  0x69, // ?? lock manufacturer commands
  0xF0,                 1,                  0x3C, //
  0x11,                 0x80,                     // Exit Sleep, then delay 150 ms
  0x29,                 0x80,                     // Main screen turn on, delay 150 ms
  0x00                                            // End of list
};

// clang-format on

/**************************************************************************/

/*!
    @brief   Initialize ST7796S chip
    Connects to the ST7796S over SPI and sends initialization procedure commands
    @param    freq  Desired SPI clock frequency
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::begin(uint32_t freq) {
  if (!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  initSPI(freq);

  if (_rst < 0) {                 // If no hardware reset pin...
    sendCommand(ST7796S_SWRESET); // Engage software reset
    delay(150);
  }

  uint8_t cmd, x, numArgs;
  const uint8_t *addr = initcmd;

  while ((cmd = pgm_read_byte(addr++)) > 0) {
    x       = pgm_read_byte(addr++);
    numArgs = x & 0x7F;
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (x & 0x80) {
      delay(150);
    }
  }

  _width  = ST7796S_TFTWIDTH;
  _height = ST7796S_TFTHEIGHT;
}

/**************************************************************************/

/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::setRotation(uint8_t m) {
  rotation = m % 4; // can't be higher than 3

  switch (rotation) {
    case 0:
      m       = (MADCTL_MX | MADCTL_BGR);
      _width  = ST7796S_TFTWIDTH;
      _height = ST7796S_TFTHEIGHT;
      break;
    case 1:
      m       = (MADCTL_MV | MADCTL_BGR);
      _width  = ST7796S_TFTHEIGHT;
      _height = ST7796S_TFTWIDTH;
      break;
    case 2:
      m       = (MADCTL_MY | MADCTL_ML | MADCTL_BGR);
      _width  = ST7796S_TFTWIDTH;
      _height = ST7796S_TFTHEIGHT;
      break;
    case 3:
      m       = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_ML | MADCTL_BGR);
      _width  = ST7796S_TFTHEIGHT;
      _height = ST7796S_TFTWIDTH;
      break;
  }

  sendCommand(ST7796S_MADCTL, &m, 1);
  setScrollMargins(0, 0); // .kbv
  scrollTo(0);
}

/**************************************************************************/

/*!
    @brief   Enable/Disable display color inversion
    @param   invert True to invert, False to have normal color
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::invertDisplay(bool invert) {
  sendCommand(invert ? ST7796S_INVON : ST7796S_INVOFF);
}

/**************************************************************************/

/*!
    @brief   Scroll display memory
    @param   y How many pixels to scroll display by
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::scrollTo(uint16_t y) {
  uint8_t data[2];

  data[0] = y >> 8;
  data[1] = y & 0xff;
  sendCommand(ST7796S_VSCRSADD, (uint8_t *)data, 2);
}

/**************************************************************************/

/*!
    @brief   Set the height of the Top and Bottom Scroll Margins
    @param   top The height of the Top scroll margin
    @param   bottom The height of the Bottom scroll margin
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::setScrollMargins(uint16_t top, uint16_t bottom) {
  // TFA+VSA+BFA must equal 480
  if (top + bottom <= ST7796S_TFTHEIGHT) {
    uint16_t middle = ST7796S_TFTHEIGHT - top - bottom;
    uint8_t  data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = middle >> 8;
    data[3] = middle & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;
    sendCommand(ST7796S_VSCRDEF, (uint8_t *)data, 6);
  }
}

/**************************************************************************/

/*!
    @brief   Set the "address window" - the rectangle we will write to RAM with
   the next chunk of      SPI data writes. The ST7796S will automatically wrap
   the data as each row is filled
    @param   x1  TFT memory 'x' origin
    @param   y1  TFT memory 'y' origin
    @param   w   Width of rectangle
    @param   h   Height of rectangle
 */

/**************************************************************************/
void Adafruit_ST7796S_kbv::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w,
                                         uint16_t h) {
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);

  writeCommand(ST7796S_CASET); // Column address set
  SPI_WRITE16(x1);
  SPI_WRITE16(x2);
  writeCommand(ST7796S_PASET); // Row address set
  SPI_WRITE16(y1);
  SPI_WRITE16(y2);
  writeCommand(ST7796S_RAMWR); // Write to RAM
}

/**************************************************************************/

/*!
    @brief  Read 8 bits of data from ST7796S configuration memory. NOT from RAM!
            This is highly undocumented/supported, it's really a hack but kinda
   works?
    @param    commandByte  The command register to read data from
    @param    index  The byte index into the command to read from
    @return   Unsigned 8-bit data read from ST7796S register
 */

/**************************************************************************/
uint8_t Adafruit_ST7796S_kbv::readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t data = 0x10 + index, ret;

  sendCommand(0xFB, &data, 1); // Set Index Register
  ret  = Adafruit_SPITFT::readcommand8(commandByte);
  data = 0x00;
  sendCommand(0xFB, &data, 1); // Set Index Register
  return ret;
}
