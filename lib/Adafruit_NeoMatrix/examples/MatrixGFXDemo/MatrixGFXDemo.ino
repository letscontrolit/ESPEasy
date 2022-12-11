// Adafruit_NeoMatrix example for single NeoPixel Shield.
// By Marc MERLIN <marc_soft@merlins.org>
// Contains code (c) Adafruit, license BSD

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Choose your prefered pixmap
//#include "heart24.h"
//#include "yellowsmiley24.h"
//#include "bluesmiley24.h"
#include "smileytongue24.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

// ESP8266 has an I2S neopixel library which can only use pin RX
// so it's recommended to use the same pin with Neopixel to avoid
// rewiring when changing libs
#ifdef ESP8266
#define PIN RX
#endif

//#define P32BY8X4
#define P16BY16X4
#if defined(P32BY8X4) || defined(P16BY16X4)
#define BM32
#endif

#ifdef BM32
#include "google32.h"
// Anything with black does not look so good with the naked eye (better on pictures)
//#include "linux32.h"
#endif

// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
#define BRIGHTNESS 32

// MATRIX DECLARATION:
// Parameter 1 = width of EACH NEOPIXEL MATRIX (not total display)
// Parameter 2 = height of each matrix
// Parameter 3 = number of matrices arranged horizontally
// Parameter 4 = number of matrices arranged vertically
// Parameter 5 = pin number (most are valid)
// Parameter 6 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
//     arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
//     EACH MATRIX proceed in the same order, or alternate lines reverse
//     direction; pick one.
//   NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
//     Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
//     two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
//   NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
//     are arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
//     (tiles) in the OVERALL DISPLAY proceed in the same order for every
//     line, or alternate lines reverse direction; pick one.  When using
//     zig-zag order, the orientation of the matrices in alternate rows
//     will be rotated 180 degrees (this is normal -- simplifies wiring).
//   See example below for these values in action.
// Parameter 7 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 pixels)
//   NEO_GRB     Pixels are wired for GRB bitstream (v2 pixels)
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA v1 pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)

#ifdef P32BY8X4
// Define full matrix width and height.
#define mw 32
#define mh 32
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(8, mh, 
  mw/8, 1, 
  PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + 
// progressive vs zigzag makes no difference for a 4 arrays next to one another
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800 );
#elif defined(P16BY16X4)
#define mw 32
#define mh 32
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(16, mh, 
  mw/16, mh/16, 
  PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + 
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_ZIGZAG,
  NEO_GRB            + NEO_KHZ800 );
#else
// Define matrix width and height.
#define mw 16
#define mh 16
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(mw, mh, 
  PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800 );
#endif

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK		0

#define LED_RED_VERYLOW 	(3 <<  11)
#define LED_RED_LOW 		(7 <<  11)
#define LED_RED_MEDIUM 		(15 << 11)
#define LED_RED_HIGH 		(31 << 11)

#define LED_GREEN_VERYLOW	(1 <<  5)   
#define LED_GREEN_LOW 		(15 << 5)  
#define LED_GREEN_MEDIUM 	(31 << 5)  
#define LED_GREEN_HIGH 		(63 << 5)  

#define LED_BLUE_VERYLOW	3
#define LED_BLUE_LOW 		7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW	(LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW		(LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM	(LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH		(LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW	(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW		(LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM		(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH		(LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW		(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

static const uint8_t PROGMEM
    mono_bmp[][8] =
    {
	{   // 0: checkered 1
	    B10101010,
	    B01010101,
	    B10101010,
	    B01010101,
	    B10101010,
	    B01010101,
	    B10101010,
	    B01010101,
			},

	{   // 1: checkered 2
	    B01010101,
	    B10101010,
	    B01010101,
	    B10101010,
	    B01010101,
	    B10101010,
	    B01010101,
	    B10101010,
			},

	{   // 2: smiley
	    B00111100,
	    B01000010,
	    B10100101,
	    B10000001,
	    B10100101,
	    B10011001,
	    B01000010,
	    B00111100 },

	{   // 3: neutral
	    B00111100,
	    B01000010,
	    B10100101,
	    B10000001,
	    B10111101,
	    B10000001,
	    B01000010,
	    B00111100 },

	{   // 4; frowny
	    B00111100,
	    B01000010,
	    B10100101,
	    B10000001,
	    B10011001,
	    B10100101,
	    B01000010,
	    B00111100 },
    };

static const uint16_t PROGMEM
    // These bitmaps were written for a backend that only supported
    // 4 bits per color with Blue/Green/Red ordering while neomatrix
    // uses native 565 color mapping as RGB.  
    // I'm leaving the arrays as is because it's easier to read
    // which color is what when separated on a 4bit boundary
    // The demo code will modify the arrays at runtime to be compatible
    // with the neomatrix color ordering and bit depth.
    RGB_bmp[][64] = {
      // 00: blue, blue/red, red, red/green, green, green/blue, blue, white
      {	0x100, 0x200, 0x300, 0x400, 0x600, 0x800, 0xA00, 0xF00, 
	0x101, 0x202, 0x303, 0x404, 0x606, 0x808, 0xA0A, 0xF0F, 
      	0x001, 0x002, 0x003, 0x004, 0x006, 0x008, 0x00A, 0x00F, 
	0x011, 0x022, 0x033, 0x044, 0x066, 0x088, 0x0AA, 0x0FF, 
	0x010, 0x020, 0x030, 0x040, 0x060, 0x080, 0x0A0, 0x0F0, 
	0x110, 0x220, 0x330, 0x440, 0x660, 0x880, 0xAA0, 0xFF0, 
	0x100, 0x200, 0x300, 0x400, 0x600, 0x800, 0xA00, 0xF00, 
	0x111, 0x222, 0x333, 0x444, 0x666, 0x888, 0xAAA, 0xFFF, },

      // 01: grey to white
      {	0x111, 0x222, 0x333, 0x555, 0x777, 0x999, 0xAAA, 0xFFF, 
	0x222, 0x222, 0x333, 0x555, 0x777, 0x999, 0xAAA, 0xFFF, 
	0x333, 0x333, 0x333, 0x555, 0x777, 0x999, 0xAAA, 0xFFF, 
	0x555, 0x555, 0x555, 0x555, 0x777, 0x999, 0xAAA, 0xFFF, 
	0x777, 0x777, 0x777, 0x777, 0x777, 0x999, 0xAAA, 0xFFF, 
	0x999, 0x999, 0x999, 0x999, 0x999, 0x999, 0xAAA, 0xFFF, 
	0xAAA, 0xAAA, 0xAAA, 0xAAA, 0xAAA, 0xAAA, 0xAAA, 0xFFF, 
	0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, },

      // 02: low red to high red
      {	0x001, 0x002, 0x003, 0x005, 0x007, 0x009, 0x00A, 0x00F, 
	0x002, 0x002, 0x003, 0x005, 0x007, 0x009, 0x00A, 0x00F, 
	0x003, 0x003, 0x003, 0x005, 0x007, 0x009, 0x00A, 0x00F, 
	0x005, 0x005, 0x005, 0x005, 0x007, 0x009, 0x00A, 0x00F, 
	0x007, 0x007, 0x007, 0x007, 0x007, 0x009, 0x00A, 0x00F, 
	0x009, 0x009, 0x009, 0x009, 0x009, 0x009, 0x00A, 0x00F, 
	0x00A, 0x00A, 0x00A, 0x00A, 0x00A, 0x00A, 0x00A, 0x00F, 
	0x00F, 0x00F, 0x00F, 0x00F, 0x00F, 0x00F, 0x00F, 0x00F, },

      // 03: low green to high green
      {	0x010, 0x020, 0x030, 0x050, 0x070, 0x090, 0x0A0, 0x0F0, 
	0x020, 0x020, 0x030, 0x050, 0x070, 0x090, 0x0A0, 0x0F0, 
	0x030, 0x030, 0x030, 0x050, 0x070, 0x090, 0x0A0, 0x0F0, 
	0x050, 0x050, 0x050, 0x050, 0x070, 0x090, 0x0A0, 0x0F0, 
	0x070, 0x070, 0x070, 0x070, 0x070, 0x090, 0x0A0, 0x0F0, 
	0x090, 0x090, 0x090, 0x090, 0x090, 0x090, 0x0A0, 0x0F0, 
	0x0A0, 0x0A0, 0x0A0, 0x0A0, 0x0A0, 0x0A0, 0x0A0, 0x0F0, 
	0x0F0, 0x0F0, 0x0F0, 0x0F0, 0x0F0, 0x0F0, 0x0F0, 0x0F0, },

      // 04: low blue to high blue
      {	0x100, 0x200, 0x300, 0x500, 0x700, 0x900, 0xA00, 0xF00, 
	0x200, 0x200, 0x300, 0x500, 0x700, 0x900, 0xA00, 0xF00, 
	0x300, 0x300, 0x300, 0x500, 0x700, 0x900, 0xA00, 0xF00, 
	0x500, 0x500, 0x500, 0x500, 0x700, 0x900, 0xA00, 0xF00, 
	0x700, 0x700, 0x700, 0x700, 0x700, 0x900, 0xA00, 0xF00, 
	0x900, 0x900, 0x900, 0x900, 0x900, 0x900, 0xA00, 0xF00, 
	0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xA00, 0xF00, 
	0xF00, 0xF00, 0xF00, 0xF00, 0xF00, 0xF00, 0xF00, 0xF00, },

      // 05: 1 black, 2R, 2O, 2G, 1B with 4 blue lines rising right
      {	0x000, 0x200, 0x000, 0x400, 0x000, 0x800, 0x000, 0xF00, 
      	0x000, 0x201, 0x002, 0x403, 0x004, 0x805, 0x006, 0xF07, 
	0x008, 0x209, 0x00A, 0x40B, 0x00C, 0x80D, 0x00E, 0xF0F, 
	0x000, 0x211, 0x022, 0x433, 0x044, 0x855, 0x066, 0xF77, 
	0x088, 0x299, 0x0AA, 0x4BB, 0x0CC, 0x8DD, 0x0EE, 0xFFF, 
	0x000, 0x210, 0x020, 0x430, 0x040, 0x850, 0x060, 0xF70, 
	0x080, 0x290, 0x0A0, 0x4B0, 0x0C0, 0x8D0, 0x0E0, 0xFF0,
	0x000, 0x200, 0x000, 0x500, 0x000, 0x800, 0x000, 0xF00, },

      // 06: 4 lines of increasing red and then green
      { 0x000, 0x000, 0x001, 0x001, 0x002, 0x002, 0x003, 0x003, 
	0x004, 0x004, 0x005, 0x005, 0x006, 0x006, 0x007, 0x007, 
	0x008, 0x008, 0x009, 0x009, 0x00A, 0x00A, 0x00B, 0x00B, 
	0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0x00F, 0x00F, 
	0x000, 0x000, 0x010, 0x010, 0x020, 0x020, 0x030, 0x030, 
	0x040, 0x040, 0x050, 0x050, 0x060, 0x060, 0x070, 0x070, 
	0x080, 0x080, 0x090, 0x090, 0x0A0, 0x0A0, 0x0B0, 0x0B0, 
	0x0C0, 0x0C0, 0x0D0, 0x0D0, 0x0E0, 0x0E0, 0x0F0, 0x0F0, },

      // 07: 4 lines of increasing red and then blue
      { 0x000, 0x000, 0x001, 0x001, 0x002, 0x002, 0x003, 0x003, 
	0x004, 0x004, 0x005, 0x005, 0x006, 0x006, 0x007, 0x007, 
	0x008, 0x008, 0x009, 0x009, 0x00A, 0x00A, 0x00B, 0x00B, 
	0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0x00F, 0x00F, 
	0x000, 0x000, 0x100, 0x100, 0x200, 0x200, 0x300, 0x300, 
	0x400, 0x400, 0x500, 0x500, 0x600, 0x600, 0x700, 0x700, 
	0x800, 0x800, 0x900, 0x900, 0xA00, 0xA00, 0xB00, 0xB00, 
	0xC00, 0xC00, 0xD00, 0xD00, 0xE00, 0xE00, 0xF00, 0xF00, },

      // 08: criss cross of green and red with diagonal blue.
      {	0xF00, 0x001, 0x003, 0x005, 0x007, 0x00A, 0x00F, 0x000, 
	0x020, 0xF21, 0x023, 0x025, 0x027, 0x02A, 0x02F, 0x020, 
	0x040, 0x041, 0xF43, 0x045, 0x047, 0x04A, 0x04F, 0x040, 
	0x060, 0x061, 0x063, 0xF65, 0x067, 0x06A, 0x06F, 0x060, 
	0x080, 0x081, 0x083, 0x085, 0xF87, 0x08A, 0x08F, 0x080, 
	0x0A0, 0x0A1, 0x0A3, 0x0A5, 0x0A7, 0xFAA, 0x0AF, 0x0A0, 
	0x0F0, 0x0F1, 0x0F3, 0x0F5, 0x0F7, 0x0FA, 0xFFF, 0x0F0, 
	0x000, 0x001, 0x003, 0x005, 0x007, 0x00A, 0x00F, 0xF00, },

      // 09: 2 lines of green, 2 red, 2 orange, 2 green
      { 0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, 
	0x0F0, 0x0F0, 0x0FF, 0x0FF, 0x00F, 0x00F, 0x0F0, 0x0F0, },

      // 10: multicolor smiley face
      { 0x000, 0x000, 0x00F, 0x00F, 0x00F, 0x00F, 0x000, 0x000, 
	0x000, 0x00F, 0x000, 0x000, 0x000, 0x000, 0x00F, 0x000, 
	0x00F, 0x000, 0xF00, 0x000, 0x000, 0xF00, 0x000, 0x00F, 
	0x00F, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00F, 
	0x00F, 0x000, 0x0F0, 0x000, 0x000, 0x0F0, 0x000, 0x00F, 
	0x00F, 0x000, 0x000, 0x0F4, 0x0F3, 0x000, 0x000, 0x00F, 
	0x000, 0x00F, 0x000, 0x000, 0x000, 0x000, 0x00F, 0x000, 
	0x000, 0x000, 0x00F, 0x00F, 0x00F, 0x00F, 0x000, 0x000, },
};


// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
    // work around "a15 cannot be used in asm here" compiler bug when using an array on ESP8266
    // uint16_t RGB_bmp_fixed[w * h];
    static uint16_t *RGB_bmp_fixed = (uint16_t *) malloc( w*h*2);
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
	uint8_t r,g,b;
	uint16_t color = pgm_read_word(bitmap + pixel);

	//Serial.print(color, HEX);
	b = (color & 0xF00) >> 8;
	g = (color & 0x0F0) >> 4;
	r = color & 0x00F;
	//Serial.print(" ");
	//Serial.print(b);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(r);
	//Serial.print(" -> ");
	// expand from 4/4/4 bits per color to 5/6/5
	b = map(b, 0, 15, 0, 31);
	g = map(g, 0, 15, 0, 63);
	r = map(r, 0, 15, 0, 31);
	//Serial.print(r);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(b);
	RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
	//Serial.print(" -> ");
	//Serial.println(RGB_bmp_fixed[pixel], HEX);
    }
    matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

// In a case of a tile of neomatrices, this test is helpful to make sure that the
// pixels are all in sequence (to check your wiring order and the tile options you
// gave to the constructor).
void count_pixels() {
    matrix->clear();
    for (uint16_t i=0; i<mh; i++) {
	for (uint16_t j=0; j<mw; j++) {
	    matrix->drawPixel(j, i, i%3==0?LED_BLUE_HIGH:i%3==1?LED_RED_HIGH:LED_GREEN_HIGH);
	    // depending on the matrix size, it's too slow to display each pixel, so
	    // make the scan init faster. This will however be too fast on a small matrix.
	    if (!(j%7)) matrix->show();
	    yield();
	}
    }
}

// Fill the screen with multiple levels of white to gauge the quality
void display_four_white() {
    matrix->clear();
    matrix->fillRect(0,0, mw,mh, LED_WHITE_HIGH);
    matrix->drawRect(1,1, mw-2,mh-2, LED_WHITE_MEDIUM);
    matrix->drawRect(2,2, mw-4,mh-4, LED_WHITE_LOW);
    matrix->drawRect(3,3, mw-6,mh-6, LED_WHITE_VERYLOW);
    matrix->show();
}

void display_bitmap(uint8_t bmp_num, uint16_t color) { 
    static uint16_t bmx,bmy;

    // Clear the space under the bitmap that will be drawn as
    // drawing a single color pixmap does not write over pixels
    // that are nul, and leaves the data that was underneath
    matrix->fillRect(bmx,bmy, bmx+8,bmy+8, LED_BLACK);
    matrix->drawBitmap(bmx, bmy, mono_bmp[bmp_num], 8, 8, color);
    bmx += 8;
    if (bmx >= mw) bmx = 0;
    if (!bmx) bmy += 8;
    if (bmy >= mh) bmy = 0;
    matrix->show();
}

void display_rgbBitmap(uint8_t bmp_num) { 
    static uint16_t bmx,bmy;

    fixdrawRGBBitmap(bmx, bmy, RGB_bmp[bmp_num], 8, 8);
    bmx += 8;
    if (bmx >= mw) bmx = 0;
    if (!bmx) bmy += 8;
    if (bmy >= mh) bmy = 0;
    matrix->show();
}

void display_lines() {
    matrix->clear();

    // 4 levels of crossing red lines.
    matrix->drawLine(0,mh/2-2, mw-1,2, LED_RED_VERYLOW);
    matrix->drawLine(0,mh/2-1, mw-1,3, LED_RED_LOW);
    matrix->drawLine(0,mh/2,   mw-1,mh/2, LED_RED_MEDIUM);
    matrix->drawLine(0,mh/2+1, mw-1,mh/2+1, LED_RED_HIGH);

    // 4 levels of crossing green lines.
    matrix->drawLine(mw/2-2, 0, mw/2-2, mh-1, LED_GREEN_VERYLOW);
    matrix->drawLine(mw/2-1, 0, mw/2-1, mh-1, LED_GREEN_LOW);
    matrix->drawLine(mw/2+0, 0, mw/2+0, mh-1, LED_GREEN_MEDIUM);
    matrix->drawLine(mw/2+1, 0, mw/2+1, mh-1, LED_GREEN_HIGH);

    // Diagonal blue line.
    matrix->drawLine(0,0, mw-1,mh-1, LED_BLUE_HIGH);
    matrix->drawLine(0,mh-1, mw-1,0, LED_ORANGE_MEDIUM);
    matrix->show();
}

void display_boxes() {
    matrix->clear();
    matrix->drawRect(0,0, mw,mh, LED_BLUE_HIGH);
    matrix->drawRect(1,1, mw-2,mh-2, LED_GREEN_MEDIUM);
    matrix->fillRect(2,2, mw-4,mh-4, LED_RED_HIGH);
    matrix->fillRect(3,3, mw-6,mh-6, LED_ORANGE_MEDIUM);
    matrix->show();
}

void display_circles() {
    matrix->clear();
    matrix->drawCircle(mw/2,mh/2, 2, LED_RED_MEDIUM);
    matrix->drawCircle(mw/2-1-min(mw,mh)/8, mh/2-1-min(mw,mh)/8, min(mw,mh)/4, LED_BLUE_HIGH);
    matrix->drawCircle(mw/2+1+min(mw,mh)/8, mh/2+1+min(mw,mh)/8, min(mw,mh)/4-1, LED_ORANGE_MEDIUM);
    matrix->drawCircle(1,mh-2, 1, LED_GREEN_LOW);
    matrix->drawCircle(mw-2,1, 1, LED_GREEN_HIGH);
    if (min(mw,mh)>12) matrix->drawCircle(mw/2-1, mh/2-1, min(mh/2-1,mw/2-1), LED_CYAN_HIGH);
    matrix->show();
}

void display_resolution() {
    matrix->setTextSize(1);
    // not wide enough;
    if (mw<16) return;
    matrix->clear();
    // Font is 5x7, if display is too small
    // 8 can only display 1 char
    // 16 can almost display 3 chars
    // 24 can display 4 chars
    // 32 can display 5 chars
    matrix->setCursor(0, 0);
    matrix->setTextColor(matrix->Color(255,0,0));
    if (mw>10) matrix->print(mw/10);
    matrix->setTextColor(matrix->Color(255,128,0)); 
    matrix->print(mw % 10);
    matrix->setTextColor(matrix->Color(0,255,0));
    matrix->print('x');
    // not wide enough to print 5 chars, go to next line
    if (mw<25) {
	if (mh==13) matrix->setCursor(6, 7);
	else if (mh>=13) {
	    matrix->setCursor(mw-11, 8);
	} else {
	    // we're not tall enough either, so we wait and display
	    // the 2nd value on top.
	    matrix->show();
	    delay(2000);
	    matrix->clear();
	    matrix->setCursor(mw-11, 0);
	}   
    }
    matrix->setTextColor(matrix->Color(0,255,128)); 
    matrix->print(mh/10);
    matrix->setTextColor(matrix->Color(0,128,255));  
    matrix->print(mh % 10);
    // enough room for a 2nd line
    if ((mw>25 && mh >14) || mh>16) {
	matrix->setCursor(0, mh-7);
	matrix->setTextColor(matrix->Color(0,255,255)); 
	if (mw>16) matrix->print('*');
	matrix->setTextColor(matrix->Color(255,0,0)); 
	matrix->print('R');
	matrix->setTextColor(matrix->Color(0,255,0));
	matrix->print('G');
	matrix->setTextColor(matrix->Color(0,0,255)); 
	matrix->print("B");
	matrix->setTextColor(matrix->Color(255,255,0)); 
	// this one could be displayed off screen, but we don't care :)
	matrix->print("*");

	// We have a big array, great, let's assume 32x32 and add something in the middle
	if (mh>24 && mw>25) {
	    for (uint16_t i=0; i<mw; i+=8) fixdrawRGBBitmap(i, mh/2-7+(i%16)/8*6, RGB_bmp[10], 8, 8);
	}
    }
    
    matrix->show();
}

void display_scrollText() {
    uint8_t size = max(int(mw/8), 1);
    matrix->clear();
    matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
    matrix->setTextSize(1);
    matrix->setRotation(0);
    for (int8_t x=7; x>=-42; x--) {
	matrix->clear();
	matrix->setCursor(x,0);
	matrix->setTextColor(LED_GREEN_HIGH);
	matrix->print("Hello");
	if (mh>11) {
	    matrix->setCursor(-20-x,mh-7);
	    matrix->setTextColor(LED_ORANGE_HIGH);
	    matrix->print("World");
	}
	matrix->show();
       delay(50);
    }

    matrix->setRotation(3);
    matrix->setTextSize(size);
    matrix->setTextColor(LED_BLUE_HIGH);
    for (int16_t x=8*size; x>=-6*8*size; x--) {
	matrix->clear();
	matrix->setCursor(x,mw/2-size*4);
	matrix->print("Rotate");
	matrix->show();
	// note that on a big array the refresh rate from show() will be slow enough that
	// the delay become irrelevant. This is already true on a 32x32 array.
        delay(50/size);
    }
    matrix->setRotation(0);
    matrix->setCursor(0,0);
    matrix->show();
}

// Scroll within big bitmap so that all if it becomes visible or bounce a small one.
// If the bitmap is bigger in one dimension and smaller in the other one, it will
// be both panned and bounced in the appropriate dimensions.
void display_panOrBounceBitmap (uint8_t bitmapSize) {
    // keep integer math, deal with values 16 times too big
    // start by showing upper left of big bitmap or centering if the display is big
    int16_t xf = max(0, (mw-bitmapSize)/2) << 4;
    int16_t yf = max(0, (mh-bitmapSize)/2) << 4;
    // scroll speed in 1/16th
    int16_t xfc = 6;
    int16_t yfc = 3;
    // scroll down and right by moving upper left corner off screen 
    // more up and left (which means negative numbers)
    int16_t xfdir = -1;
    int16_t yfdir = -1;

    for (uint16_t i=1; i<200; i++) {
	bool updDir = false;

	// Get actual x/y by dividing by 16.
	int16_t x = xf >> 4;
	int16_t y = yf >> 4;

	matrix->clear();
	// bounce 8x8 tri color smiley face around the screen
	if (bitmapSize == 8) fixdrawRGBBitmap(x, y, RGB_bmp[10], 8, 8);
	// pan 24x24 pixmap
	if (bitmapSize == 24) matrix->drawRGBBitmap(x, y, (const uint16_t *) bitmap24, bitmapSize, bitmapSize);
#ifdef BM32
	if (bitmapSize == 32) matrix->drawRGBBitmap(x, y, (const uint16_t *) bitmap32, bitmapSize, bitmapSize);
#endif
	matrix->show();
	 
	// Only pan if the display size is smaller than the pixmap
	// but not if the difference is too small or it'll look bad.
	if (bitmapSize-mw>2) {
	    xf += xfc*xfdir;
	    if (xf >= 0)                      { xfdir = -1; updDir = true ; };
	    // we don't go negative past right corner, go back positive
	    if (xf <= ((mw-bitmapSize) << 4)) { xfdir = 1;  updDir = true ; };
	}
	if (bitmapSize-mh>2) {
	    yf += yfc*yfdir;
	    // we shouldn't display past left corner, reverse direction.
	    if (yf >= 0)                      { yfdir = -1; updDir = true ; };
	    if (yf <= ((mh-bitmapSize) << 4)) { yfdir = 1;  updDir = true ; };
	}
	// only bounce a pixmap if it's smaller than the display size
	if (mw>bitmapSize) {
	    xf += xfc*xfdir;
	    // Deal with bouncing off the 'walls'
	    if (xf >= (mw-bitmapSize) << 4) { xfdir = -1; updDir = true ; };
	    if (xf <= 0)           { xfdir =  1; updDir = true ; };
	}
	if (mh>bitmapSize) {
	    yf += yfc*yfdir;
	    if (yf >= (mh-bitmapSize) << 4) { yfdir = -1; updDir = true ; };
	    if (yf <= 0)           { yfdir =  1; updDir = true ; };
	}
	
	if (updDir) {
	    // Add -1, 0 or 1 but bind result to 1 to 1.
	    // Let's take 3 is a minimum speed, otherwise it's too slow.
	    xfc = constrain(xfc + random(-1, 2), 3, 16);
	    yfc = constrain(xfc + random(-1, 2), 3, 16);
	}
	delay(10);
    }
}


void loop() {
    // clear the screen after X bitmaps have been displayed and we
    // loop back to the top left corner
    // 8x8 => 1, 16x8 => 2, 17x9 => 6
    static uint8_t pixmap_count = ((mw+7)/8) * ((mh+7)/8);

// You can't use millis to time frame fresh rate because it uses cli() which breaks millis()
// So I use my stopwatch to count 200 displays and that's good enough
#if 0
    // 200 displays in 13 seconds = 15 frames per second for 4096 pixels
    for (uint8_t i=0; i<100; i++) { 
	matrix->fillScreen(LED_BLUE_LOW);
	matrix->show();
	matrix->fillScreen(LED_RED_LOW);
	matrix->show();
    }
#endif

    count_pixels();
    delay(1000);

    display_four_white();
    delay(3000);

    Serial.print("Screen pixmap capacity: ");
    Serial.println(pixmap_count);

    // multicolor bitmap sent as many times as we can display an 8x8 pixmap
    for (uint8_t i=0; i<=pixmap_count; i++)
    {
	display_rgbBitmap(0);
    }
    delay(1000);

    display_resolution();
    delay(3000);

    // Cycle through red, green, blue, display 2 checkered patterns
    // useful to debug some screen types and alignment.
    uint16_t bmpcolor[] = { LED_GREEN_HIGH, LED_BLUE_HIGH, LED_RED_HIGH };
    for (uint8_t i=0; i<3; i++)
    {
	display_bitmap(0, bmpcolor[i]);
 	delay(500);
	display_bitmap(1, bmpcolor[i]);
 	delay(500);
    }

    // Display 3 smiley faces.
    for (uint8_t i=2; i<=4; i++)
    {
	display_bitmap(i, bmpcolor[i-2]);
	// If more than one pixmap displayed per screen, display more quickly.
	delay(mw>8?500:1500);
    }
    // If we have multiple pixmaps displayed at once, wait a bit longer on the last.
    delay(mw>8?1000:500);

    display_lines();
    delay(3000);

    display_boxes();
    delay(3000);

    display_circles();
    matrix->clear();
    delay(3000);

    for (uint8_t i=0; i<=(sizeof(RGB_bmp)/sizeof(RGB_bmp[0])-1); i++)
    {
	display_rgbBitmap(i);
	delay(mw>8?500:1500);
    }
    // If we have multiple pixmaps displayed at once, wait a bit longer on the last.
    delay(mw>8?1000:500);

    display_scrollText();

#ifdef BM32
    display_panOrBounceBitmap(32);
#endif
    // pan a big pixmap
    display_panOrBounceBitmap(24);
    // bounce around a small one
    display_panOrBounceBitmap(8);
}

void setup() {
    Serial.begin(115200);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(BRIGHTNESS);
    // Test full bright of all LEDs. If brightness is too high
    // for your current limit (i.e. USB), decrease it.
    matrix->fillScreen(LED_WHITE_HIGH);
    matrix->show();
    delay(3000);
    matrix->clear();
}

// vim:sts=4:sw=4
