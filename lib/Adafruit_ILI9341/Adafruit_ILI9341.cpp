/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
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

#include "Adafruit_ILI9341.h"
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void) {
#if defined(ESP8266) || defined (ARDUINO_ARCH_ARC32)
  // max speed!
  SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));
#else
    // max speed!
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
#endif
}
static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void) {
  SPI.endTransaction();
}
#else
#define spi_begin()
#define spi_end()
#endif


// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t mosi,
				   int8_t sclk, int8_t rst, int8_t miso) : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _mosi  = mosi;
  _miso = miso;
  _sclk = sclk;
  _rst  = rst;
  hwSPI = false;
}


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  hwSPI = true;
  _mosi  = _sclk = 0;
}

void Adafruit_ILI9341::spiwrite(uint8_t c) {

  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

  if (hwSPI) {
#if defined (__AVR__)
  #ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
  #endif
    SPDR = c;
    while(!(SPSR & _BV(SPIF)));
  #ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
  #endif
#else
    SPI.transfer(c);
#endif
  } else {
#if defined(ESP8266) || defined (ARDUINO_ARCH_ARC32)
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) {
	digitalWrite(_mosi, HIGH); 
      } else {
	digitalWrite(_mosi, LOW); 
      }
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
    }
#else
    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) {
	//digitalWrite(_mosi, HIGH); 
	*mosiport |=  mosipinmask;
      } else {
	//digitalWrite(_mosi, LOW); 
	*mosiport &= ~mosipinmask;
      }
      //digitalWrite(_sclk, HIGH);
      *clkport |=  clkpinmask;
      //digitalWrite(_sclk, LOW);
      *clkport &= ~clkpinmask;
    }
#endif
  }
}


void Adafruit_ILI9341::writecommand(uint8_t c) {
#if defined (USE_FAST_PINIO)
  *dcport &= ~dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, LOW);
  digitalWrite(_sclk, LOW);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined (USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
}


void Adafruit_ILI9341::writedata(uint8_t c) {
#if defined (USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined (USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
} 


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ILI9341::commandList(uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


void Adafruit_ILI9341::begin(void) {
  if (_rst > 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
  }

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined (USE_FAST_PINIO)
  csport    = portOutputRegister(digitalPinToPort(_cs));
  cspinmask = digitalPinToBitMask(_cs);
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  dcpinmask = digitalPinToBitMask(_dc);
#endif

  if(hwSPI) { // Using hardware SPI
    SPI.begin();

#ifndef SPI_HAS_TRANSACTION
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
  #if defined (_AVR__)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
    mySPCR = SPCR;
  #elif defined(TEENSYDUINO)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
  #elif defined (__arm__)
    SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
  #endif
#endif
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

#if defined (USE_FAST_PINIO)
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    clkpinmask  = digitalPinToBitMask(_sclk);
    mosiport    = portOutputRegister(digitalPinToPort(_mosi));
    mosipinmask = digitalPinToBitMask(_mosi);
    *clkport   &= ~clkpinmask;
    *mosiport  &= ~mosipinmask;
#endif
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
  uint8_t x = readcommand8(ILI9341_RDMODE);
  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDMADCTL);
  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDPIXFMT);
  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDIMGFMT);
  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
*/
  //if(cmdList) commandList(cmdList);
  
  if (hwSPI) spi_begin();
  writecommand(0xEF);
  writedata(0x03);
  writedata(0x80);
  writedata(0x02);

  writecommand(0xCF);  
  writedata(0x00); 
  writedata(0XC1); 
  writedata(0X30); 

  writecommand(0xED);  
  writedata(0x64); 
  writedata(0x03); 
  writedata(0X12); 
  writedata(0X81); 
 
  writecommand(0xE8);  
  writedata(0x85); 
  writedata(0x00); 
  writedata(0x78); 

  writecommand(0xCB);  
  writedata(0x39); 
  writedata(0x2C); 
  writedata(0x00); 
  writedata(0x34); 
  writedata(0x02); 
 
  writecommand(0xF7);  
  writedata(0x20); 

  writecommand(0xEA);  
  writedata(0x00); 
  writedata(0x00); 
 
  writecommand(ILI9341_PWCTR1);    //Power control 
  writedata(0x23);   //VRH[5:0] 
 
  writecommand(ILI9341_PWCTR2);    //Power control 
  writedata(0x10);   //SAP[2:0];BT[3:0] 
 
  writecommand(ILI9341_VMCTR1);    //VCM control 
  writedata(0x3e); //¶Ô±È¶Èµ÷½Ú
  writedata(0x28); 
  
  writecommand(ILI9341_VMCTR2);    //VCM control2 
  writedata(0x86);  //--
 
  writecommand(ILI9341_MADCTL);    // Memory Access Control 
  writedata(0x48);

  writecommand(ILI9341_PIXFMT);    
  writedata(0x55); 
  
  writecommand(ILI9341_FRMCTR1);    
  writedata(0x00);  
  writedata(0x18); 
 
  writecommand(ILI9341_DFUNCTR);    // Display Function Control 
  writedata(0x08); 
  writedata(0x82);
  writedata(0x27);  
 
  writecommand(0xF2);    // 3Gamma Function Disable 
  writedata(0x00); 
 
  writecommand(ILI9341_GAMMASET);    //Gamma curve selected 
  writedata(0x01); 
 
  writecommand(ILI9341_GMCTRP1);    //Set Gamma 
  writedata(0x0F); 
  writedata(0x31); 
  writedata(0x2B); 
  writedata(0x0C); 
  writedata(0x0E); 
  writedata(0x08); 
  writedata(0x4E); 
  writedata(0xF1); 
  writedata(0x37); 
  writedata(0x07); 
  writedata(0x10); 
  writedata(0x03); 
  writedata(0x0E); 
  writedata(0x09); 
  writedata(0x00); 
  
  writecommand(ILI9341_GMCTRN1);    //Set Gamma 
  writedata(0x00); 
  writedata(0x0E); 
  writedata(0x14); 
  writedata(0x03); 
  writedata(0x11); 
  writedata(0x07); 
  writedata(0x31); 
  writedata(0xC1); 
  writedata(0x48); 
  writedata(0x08); 
  writedata(0x0F); 
  writedata(0x0C); 
  writedata(0x31); 
  writedata(0x36); 
  writedata(0x0F); 

  writecommand(ILI9341_SLPOUT);    //Exit Sleep 
  if (hwSPI) spi_end();
  delay(120); 		
  if (hwSPI) spi_begin();
  writecommand(ILI9341_DISPON);    //Display on 
  if (hwSPI) spi_end();

}


void Adafruit_ILI9341::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) {

  writecommand(ILI9341_CASET); // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);     // XSTART 
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);     // XEND

  writecommand(ILI9341_PASET); // Row addr set
  writedata(y0>>8);
  writedata(y0);     // YSTART
  writedata(y1>>8);
  writedata(y1);     // YEND

  writecommand(ILI9341_RAMWR); // write to RAM
}


void Adafruit_ILI9341::pushColor(uint16_t color) {
  if (hwSPI) spi_begin();

#if defined(USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(color >> 8);
  spiwrite(color);

#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif

  if (hwSPI) spi_end();
}

void Adafruit_ILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  if (hwSPI) spi_begin();
  setAddrWindow(x,y,x+1,y+1);

#if defined(USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(color >> 8);
  spiwrite(color);

#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif

  if (hwSPI) spi_end();
}


void Adafruit_ILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;

  if((y+h-1) >= _height) 
    h = _height-y;

  if (hwSPI) spi_begin();
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }

#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif

  if (hwSPI) spi_end();
}


void Adafruit_ILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  if (hwSPI) spi_begin();
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;
#if defined(USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif
  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }
#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
  if (hwSPI) spi_end();
}

void Adafruit_ILI9341::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  if (hwSPI) spi_begin();
  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif

  if (hwSPI) spi_end();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ILI9341::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ILI9341::setRotation(uint8_t m) {

  if (hwSPI) spi_begin();
  writecommand(ILI9341_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(MADCTL_MX | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
     break;
   case 1:
     writedata(MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  case 2:
    writedata(MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;
   case 3:
     writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  }
  if (hwSPI) spi_end();
}


void Adafruit_ILI9341::invertDisplay(boolean i) {
  if (hwSPI) spi_begin();
  writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
  if (hwSPI) spi_end();
}


////////// stuff not actively being used, but kept for posterity


uint8_t Adafruit_ILI9341::spiread(void) {
  uint8_t r = 0;

  if (hwSPI) {
#if defined (__AVR__)
  #ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
  #endif
    SPDR = 0x00;
    while(!(SPSR & _BV(SPIF)));
    r = SPDR;

  #ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
  #endif
#else
    r = SPI.transfer(0x00);
#endif

  } else {

    for (uint8_t i=0; i<8; i++) {
      digitalWrite(_sclk, LOW);
      digitalWrite(_sclk, HIGH);
      r <<= 1;
      if (digitalRead(_miso))
	r |= 0x1;
    }
  }
  //Serial.print("read: 0x"); Serial.print(r, HEX);
  
  return r;
}

 uint8_t Adafruit_ILI9341::readdata(void) {
   digitalWrite(_dc, HIGH);
   digitalWrite(_cs, LOW);
   uint8_t r = spiread();
   digitalWrite(_cs, HIGH);
   
   return r;
}
 

uint8_t Adafruit_ILI9341::readcommand8(uint8_t c, uint8_t index) {
   if (hwSPI) spi_begin();
   digitalWrite(_dc, LOW); // command
   digitalWrite(_cs, LOW);
   spiwrite(0xD9);  // woo sekret command?
   digitalWrite(_dc, HIGH); // data
   spiwrite(0x10 + index);
   digitalWrite(_cs, HIGH);

   digitalWrite(_dc, LOW);
   digitalWrite(_sclk, LOW);
   digitalWrite(_cs, LOW);
   spiwrite(c);
 
   digitalWrite(_dc, HIGH);
   uint8_t r = spiread();
   digitalWrite(_cs, HIGH);
   if (hwSPI) spi_end();
   return r;
}
void Adafruit_ILI9341::clearScreen()
{
  fillScreen(ILI9341_BLACK);
}

 
/*

 uint16_t Adafruit_ILI9341::readcommand16(uint8_t c) {
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
 
 uint32_t Adafruit_ILI9341::readcommand32(uint8_t c) {
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
