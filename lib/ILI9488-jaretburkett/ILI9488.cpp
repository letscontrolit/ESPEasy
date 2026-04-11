/***************************************************
   STM32 Support added by Jaret Burkett at OSHlab.com

   This is our library for the Adafruit ILI9488 Breakout and Shield
   ----> http://www.adafruit.com/products/1651

   Check out the links above for our tutorials and wiring diagrams
   These displays use SPI to communicate, 4 or 5 pins are required to
   interface (RST is optional)
   Adafruit invests time and resources providing this open source code,
   please support Adafruit and open-source hardware by purchasing
   products from Adafruit!

   Written by Limor Fried/Ladyada for Adafruit Industries.
   MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "ILI9488.h"
#ifdef __AVR
  # include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  # include <pgmspace.h>
#endif // ifdef __AVR

#ifndef ARDUINO_STM32_FEATHER
  # include "pins_arduino.h"
  # include "wiring_private.h"
#endif // ifndef ARDUINO_STM32_FEATHER

#include <limits.h>
#include <SPI.h>


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void) {
# if defined(ARDUINO_ARCH_ARC32)

  // max speed!
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
# else // if defined(ARDUINO_ARCH_ARC32)

  // max speed!
  SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
# endif // if defined(ARDUINO_ARCH_ARC32)
}

static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void) {
  SPI.endTransaction();
}

#else // ifdef SPI_HAS_TRANSACTION
# define spi_begin()
# define spi_end()
#endif // ifdef SPI_HAS_TRANSACTION

// Constructor when using software SPI.  All output pins are configurable.
ILI9488::ILI9488(int8_t cs, int8_t dc, int8_t mosi,
                 int8_t sclk, int8_t rst, int8_t miso) : Adafruit_GFX(ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _mosi = mosi;
  _miso = miso;
  _sclk = sclk;
  _rst  = rst;
  hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
ILI9488::ILI9488(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  hwSPI = true;
  _mosi = _sclk = 0;
}

void ILI9488::spiwrite(uint8_t c) {
  // Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

  if (hwSPI) {
#if defined(__AVR__)
  # ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
  # endif // ifndef SPI_HAS_TRANSACTION
    SPDR = c;

    while (!(SPSR & _BV(SPIF))) {}
  # ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
  # endif // ifndef SPI_HAS_TRANSACTION
#else // if defined(__AVR__)
    SPI.transfer(c);
#endif // if defined(__AVR__)
  } else {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_ARC32)

    for (uint8_t bit = 0x80; bit; bit >>= 1) {
      if (c & bit) {
        digitalWrite(_mosi, HIGH);
      } else {
        digitalWrite(_mosi, LOW);
      }
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
    }
#else // if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_ARC32)

    // Fast SPI bitbang swiped from LPD8806 library
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
      if (c & bit) {
        // digitalWrite(_mosi, HIGH);
        *mosiport |=  mosipinmask;
      } else {
        // digitalWrite(_mosi, LOW);
        *mosiport &= ~mosipinmask;
      }

      // digitalWrite(_sclk, HIGH);
      *clkport |=  clkpinmask;

      // digitalWrite(_sclk, LOW);
      *clkport &= ~clkpinmask;
    }
#endif // if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_ARC32)
  }
}

void ILI9488::writecommand(uint8_t c) {
#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport &= ~dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc,   LOW);
  digitalWrite(_sclk, LOW);
  digitalWrite(_cs,   LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  spiwrite(c);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
}

void ILI9488::writedata(uint8_t c) {
#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  spiwrite(c);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void ILI9488::commandList(uint8_t *addr) {
  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow

  while (numCommands--) {                // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit

    while (numArgs--) {                  //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if (ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)

      if (ms == 255) { ms = 500;  // If 255, delay for 500 ms
      }
      delay(ms);
    }
  }
}

void ILI9488::begin(void) {
  if (_rst > 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
  }

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  csport    = portOutputRegister(digitalPinToPort(_cs));
  cspinmask = digitalPinToBitMask(_cs);
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  dcpinmask = digitalPinToBitMask(_dc);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { // Using hardware SPI
    SPI.begin();

#ifndef SPI_HAS_TRANSACTION
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
  # if defined(_AVR__)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
    mySPCR = SPCR;
  # elif defined(TEENSYDUINO) || defined(__STM32F1__)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
  # elif defined(__arm__)
    SPI.setClockDivider(11);             // 8-ish MHz (full! speed!)
  # endif // if defined(_AVR__)
#endif // ifndef SPI_HAS_TRANSACTION
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    clkpinmask  = digitalPinToBitMask(_sclk);
    mosiport    = portOutputRegister(digitalPinToPort(_mosi));
    mosipinmask = digitalPinToBitMask(_mosi);
    *clkport   &= ~clkpinmask;
    *mosiport  &= ~mosipinmask;
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  }

  // toggle RST low to reset
  if (_rst > 0) {
    digitalWrite(_rst, HIGH);
    delay(5);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(150);
  }

  /*
     uint8_t x = readcommand8(ILI9488_RDMODE);
     Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
     x = readcommand8(ILI9488_RDMADCTL);
     Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
     x = readcommand8(ILI9488_RDPIXFMT);
     Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
     x = readcommand8(ILI9488_RDIMGFMT);
     Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
     x = readcommand8(ILI9488_RDSELFDIAG);
     Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
   */

  // if(cmdList) commandList(cmdList);

  if (hwSPI) { spi_begin(); }

  writecommand(ILI9488_GMCTRP1);
  writedata(0x00);
  writedata(0x03);
  writedata(0x09);
  writedata(0x08);
  writedata(0x16);
  writedata(0x0A);
  writedata(0x3F);
  writedata(0x78);
  writedata(0x4C);
  writedata(0x09);
  writedata(0x0A);
  writedata(0x08);
  writedata(0x16);
  writedata(0x1A);
  writedata(0x0F);


  writecommand(ILI9488_GMCTRN1);
  writedata(0x00);
  writedata(0x16);
  writedata(0x19);
  writedata(0x03);
  writedata(0x0F);
  writedata(0x05);
  writedata(0x32);
  writedata(0x45);
  writedata(0x46);
  writedata(0x04);
  writedata(0x0E);
  writedata(0x0D);
  writedata(0x35);
  writedata(0x37);
  writedata(0x0F);


  writecommand(ILI9488_PWCTR1);  // Power Control 1
  writedata(0x17);               // Vreg1out
  writedata(0x15);               // Verg2out

  writecommand(ILI9488_PWCTR2);  // Power Control 2
  writedata(0x41);               // VGH,VGL

  writecommand(ILI9488_VMCTR1);  // Power Control 3
  writedata(0x00);
  writedata(0x12);               // Vcom
  writedata(0x80);

  writecommand(ILI9488_MADCTL);  // Memory Access
  writedata(0x48);

  writecommand(ILI9488_PIXFMT);  // Interface Pixel Format
  writedata(0x66);               // 18 bit

  writecommand(0XB0);            // Interface Mode Control
  writedata(0x80);               // SDO NOT USE

  writecommand(ILI9488_FRMCTR1); // Frame rate
  writedata(0xA0);               // 60Hz

  writecommand(ILI9488_INVCTR);  // Display Inversion Control
  writedata(0x02);               // 2-dot

  writecommand(ILI9488_DFUNCTR); // Display Function Control  RGB/MCU Interface Control
  writedata(0x02);               // MCU
  writedata(0x02);               // Source,Gate scan dieection

  writecommand(0XE9);            // Set Image Functio
  writedata(0x00);               // Disable 24 bit data

  writecommand(0xF7);            // Adjust Control
  writedata(0xA9);
  writedata(0x51);
  writedata(0x2C);
  writedata(0x82);              // D7 stream, loose


  writecommand(ILI9488_SLPOUT); // Exit Sleep

  if (hwSPI) { spi_end(); }
  delay(120);

  if (hwSPI) { spi_begin(); }
  writecommand(ILI9488_DISPON); // Display on

  if (hwSPI) { spi_end(); }
}

void ILI9488::setScrollArea(uint16_t topFixedArea, uint16_t bottomFixedArea) {
  if (hwSPI) { spi_begin(); }
  writecommand(0x33); // Vertical scroll definition
  writedata(topFixedArea >> 8);
  writedata(topFixedArea);
  writedata((_height - topFixedArea - bottomFixedArea) >> 8);
  writedata(_height - topFixedArea - bottomFixedArea);
  writedata(bottomFixedArea >> 8);
  writedata(bottomFixedArea);

  if (hwSPI) { spi_end(); }
}

void ILI9488::scroll(uint16_t pixels) {
  if (hwSPI) { spi_begin(); }
  writecommand(0x37); // Vertical scrolling start address
  writedata(pixels >> 8);
  writedata(pixels);

  if (hwSPI) { spi_end(); }
}

void ILI9488::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                            uint16_t y1) {
  writecommand(ILI9488_CASET); // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);        // XSTART
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);        // XEND

  writecommand(ILI9488_PASET); // Row addr set
  writedata(y0 >> 8);
  writedata(y0 & 0xff);        // YSTART
  writedata(y1 >> 8);
  writedata(y1 & 0xff);        // YEND

  writecommand(ILI9488_RAMWR); // write to RAM
}

void ILI9488::drawImage(const uint8_t *img, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height)) { return; }

  if ((x + w - 1) >= _width) { w = _width  - x; }

  if ((y + h - 1) >= _height) { h = _height - y; }

  if (hwSPI) { spi_begin(); }
  setAddrWindow(x, y, x + w - 1, y + h - 1);

  // uint8_t hi = color >> 8, lo = color;

  #if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
  #else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
  #endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  uint8_t  linebuff[w * 3 + 1];

  uint32_t count = 0;

  for (uint16_t i = 0; i < h; i++) {
    uint16_t pixcount = 0;

    for (uint16_t o = 0; o <  w; o++) {
      uint8_t b1 = img[count];
      count++;
      uint8_t b2 = img[count];
      count++;
      uint16_t color = b1 << 8 | b2;
      linebuff[pixcount] = (((color & 0xF800) >> 11) * 255) / 31;
      pixcount++;
      linebuff[pixcount] = (((color & 0x07E0) >> 5) * 255) / 63;
      pixcount++;
      linebuff[pixcount] = ((color & 0x001F) * 255) / 31;
      pixcount++;
    } // for row
    #if defined(__STM32F1__)
    SPI.dmaSend(linebuff, w * 3);
    #else // if defined(__STM32F1__)

    for (uint16_t b = 0; b < w * 3; b++) {
      spiwrite(linebuff[b]);
    }
    #endif // if defined(__STM32F1__)
  } // for col
  #if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
  #else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
  #endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::pushColor(uint16_t color) {
  if (hwSPI) { spi_begin(); }

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  // spiwrite(color >> 8);
  // spiwrite(color);
  // spiwrite(0); // added for 24 bit
  write16BitColor(color);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::pushColors(uint16_t *data, uint8_t len, boolean first) {
  uint16_t color;
  uint8_t  buff[len * 3 + 1];
  uint16_t count    = 0;
  uint8_t  lencount = len;

  if (hwSPI) { spi_begin(); }
  #if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport &= ~cspinmask;
  #else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, LOW);
  #endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (first == true) { // Issue GRAM write command only on first call
    #if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    #else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
    digitalWrite(_dc, HIGH);
    #endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  }

  while (lencount--) {
    color       = *data++;
    buff[count] = (((color & 0xF800) >> 11) * 255) / 31;
    count++;
    buff[count] = (((color & 0x07E0) >> 5) * 255) / 63;
    count++;
    buff[count] = ((color & 0x001F) * 255) / 31;
    count++;
  }
  #if defined(__STM32F1__)
  SPI.dmaSend(buff, len * 3);
  #else // if defined(__STM32F1__)

  for (uint16_t b = 0; b < len * 3; b++) {
    spiwrite(buff[b]);
  }
  #endif // if defined(__STM32F1__)
  #if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
  #else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
  #endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::write16BitColor(uint16_t color) {
  // #if (__STM32F1__)
  //     uint8_t buff[4] = {
  //       (((color & 0xF800) >> 11)* 255) / 31,
  //       (((color & 0x07E0) >> 5) * 255) / 63,
  //       ((color & 0x001F)* 255) / 31
  //     };
  //     SPI.dmaSend(buff, 3);
  // #else
  uint8_t r = (color & 0xF800) >> 11;
  uint8_t g = (color & 0x07E0) >> 5;
  uint8_t b = color & 0x001F;

  r = (r * 255) / 31;
  g = (g * 255) / 63;
  b = (b * 255) / 31;

  spiwrite(r);
  spiwrite(g);
  spiwrite(b);

  // #endif
}

void ILI9488::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) { return; }

  if (hwSPI) { spi_begin(); }
  setAddrWindow(x, y, x + 1, y + 1);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  // spiwrite(color >> 8);
  // spiwrite(color);
  // spiwrite(0); // added for 24 bit
  write16BitColor(color);

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::drawFastVLine(int16_t x, int16_t y, int16_t h,
                            uint16_t color) {
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height)) { return; }

  if ((y + h - 1) >= _height) {
    h = _height - y;
  }

  if (hwSPI) { spi_begin(); }
  setAddrWindow(x, y, x, y + h - 1);

  //  uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  while (h--) {
    // spiwrite(hi);
    // spiwrite(lo);
    // spiwrite(0); // added for 24 bit
    write16BitColor(color);
  }

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::drawFastHLine(int16_t x, int16_t y, int16_t w,
                            uint16_t color) {
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height)) { return; }

  if ((x + w - 1) >= _width) { w = _width - x; }

  if (hwSPI) { spi_begin(); }
  setAddrWindow(x, y, x + w - 1, y);

  // uint8_t hi = color >> 8, lo = color;
#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  while (w--) {
    // spiwrite(hi);
    // spiwrite(lo);
    // spiwrite(0); // added for 24 bit
    write16BitColor(color);
  }
#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

void ILI9488::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}

// fill a rectangle
void ILI9488::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color) {
  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height)) { return; }

  if ((x + w - 1) >= _width) { w = _width  - x; }

  if ((y + h - 1) >= _height) { h = _height - y; }

  if (hwSPI) { spi_begin(); }
  setAddrWindow(x, y, x + w - 1, y + h - 1);

  // uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
#if (__STM32F1__)

  // use dma fast fills
  uint8_t buff[4] = {
    (((color & 0xF800) >> 11) * 255) / 31,
    (((color & 0x07E0) >> 5) * 255) / 63,
    ((color & 0x001F) * 255) / 31
  };
  uint8_t linebuff[w * 3 + 1];
  int     cnt = 0;

  for (int i = 0; i < w; i++) {
    linebuff[cnt] = buff[0];
    cnt++;
    linebuff[cnt] = buff[1];
    cnt++;
    linebuff[cnt] = buff[2];
    cnt++;
  }

  for (y = h; y > 0; y--) {
    SPI.dmaSend(linebuff, w * 3);
  }
#else // if (__STM32F1__)

  for (y = h; y > 0; y--) {
    for (x = w; x > 0; x--) {
      // spiwrite(hi);
      // spiwrite(lo);
      // spiwrite(0); // added for 24 bit
      write16BitColor(color);
    }
  }
#endif // if (__STM32F1__)
#if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  *csport |= cspinmask;
#else // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)
  digitalWrite(_cs, HIGH);
#endif // if defined(USE_FAST_PINIO) && !defined(_VARIANT_ARDUINO_STM32_)

  if (hwSPI) { spi_end(); }
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ILI9488::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void ILI9488::setRotation(uint8_t m) {
  if (hwSPI) { spi_begin(); }
  writecommand(ILI9488_MADCTL);
  rotation = m % 4; // can't be higher than 3

  switch (rotation) {
    case 0:
      writedata(MADCTL_MX | MADCTL_BGR);
      _width  = ILI9488_TFTWIDTH;
      _height = ILI9488_TFTHEIGHT;
      break;
    case 1:
      writedata(MADCTL_MV | MADCTL_BGR);
      _width  = ILI9488_TFTHEIGHT;
      _height = ILI9488_TFTWIDTH;
      break;
    case 2:
      writedata(MADCTL_MY | MADCTL_BGR);
      _width  = ILI9488_TFTWIDTH;
      _height = ILI9488_TFTHEIGHT;
      break;
    case 3:
      writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      _width  = ILI9488_TFTHEIGHT;
      _height = ILI9488_TFTWIDTH;
      break;
  }

  if (hwSPI) { spi_end(); }
}

void ILI9488::invertDisplay(boolean i) {
  if (hwSPI) { spi_begin(); }
  writecommand(i ? ILI9488_INVON : ILI9488_INVOFF);

  if (hwSPI) { spi_end(); }
}

////////// stuff not actively being used, but kept for posterity


uint8_t ILI9488::spiread(void) {
  uint8_t r = 0;

  if (hwSPI) {
#if defined(__AVR__)
  # ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
  # endif // ifndef SPI_HAS_TRANSACTION
    SPDR = 0x00;

    while (!(SPSR & _BV(SPIF))) {}
    r = SPDR;

  # ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
  # endif // ifndef SPI_HAS_TRANSACTION
#else // if defined(__AVR__)
    r = SPI.transfer(0x00);
#endif // if defined(__AVR__)
  } else {
    for (uint8_t i = 0; i < 8; i++) {
      digitalWrite(_sclk, LOW);
      digitalWrite(_sclk, HIGH);
      r <<= 1;

      if (digitalRead(_miso)) {
        r |= 0x1;
      }
    }
  }

  // Serial.print("read: 0x"); Serial.print(r, HEX);

  return r;
}

uint8_t ILI9488::readdata(void) {
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
  uint8_t r = spiread();

  digitalWrite(_cs, HIGH);

  return r;
}

uint8_t ILI9488::readcommand8(uint8_t c, uint8_t index) {
  if (hwSPI) { spi_begin(); }
  digitalWrite(_dc, LOW);  // command
  digitalWrite(_cs, LOW);
  spiwrite(0xD9);          // woo sekret command?
  digitalWrite(_dc, HIGH); // data
  spiwrite(0x10 + index);
  digitalWrite(_cs,   HIGH);

  digitalWrite(_dc,   LOW);
  digitalWrite(_sclk, LOW);
  digitalWrite(_cs,   LOW);
  spiwrite(c);

  digitalWrite(_dc, HIGH);
  uint8_t r = spiread();

  digitalWrite(_cs, HIGH);

  if (hwSPI) { spi_end(); }
  return r;
}

/*

   uint16_t ILI9488::readcommand16(uint8_t c) {
   digitalWrite(_dc, LOW);
   if (_cs)
   digitalWrite(_cs, LOW);

   spiwrite(c);
   pinMode(_sid, INPUT); // input!
   uint16_t r = spiread();
   r <<= 8;
   r |= spiread();
   if (_cs)
   digitalWrite(_cs, HIGH);

   pinMode(_sid, OUTPUT); // back to output
   return r;
   }

   uint32_t ILI9488::readcommand32(uint8_t c) {
   digitalWrite(_dc, LOW);
   if (_cs)
   digitalWrite(_cs, LOW);
   spiwrite(c);
   pinMode(_sid, INPUT); // input!

   dummyclock();
   dummyclock();

   uint32_t r = spiread();
   r <<= 8;
   r |= spiread();
   r <<= 8;
   r |= spiread();
   r <<= 8;
   r |= spiread();
   if (_cs)
   digitalWrite(_cs, HIGH);

   pinMode(_sid, OUTPUT); // back to output
   return r;
   }

 */
