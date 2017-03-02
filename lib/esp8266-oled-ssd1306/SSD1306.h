/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch

Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
*/
#pragma once

#include <Arduino.h>
#include "SSD1306Fonts.h"

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define WIDTH_POS 0
#define HEIGHT_POS 1
#define FIRST_CHAR_POS 2
#define CHAR_NUM_POS 3
#define CHAR_WIDTH_START_POS 4

#define TEXT_ALIGN_LEFT 0
#define TEXT_ALIGN_CENTER 1
#define TEXT_ALIGN_RIGHT 2

#define CHARGEPUMP 0x8D
#define COLUMNADDR 0x21
#define COMSCANDEC 0xC8
#define COMSCANINC 0xC0
#define DISPLAYALLON 0xA5
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define EXTERNALVCC 0x1
#define INVERTDISPLAY 0xA7
#define MEMORYMODE 0x20
#define NORMALDISPLAY 0xA6
#define PAGEADDR 0x22
#define SEGREMAP 0xA0
#define SETCOMPINS 0xDA
#define SETCONTRAST 0x81
#define SETDISPLAYCLOCKDIV 0xD5
#define SETDISPLAYOFFSET 0xD3
#define SETHIGHCOLUMN 0x10
#define SETLOWCOLUMN 0x00
#define SETMULTIPLEX 0xA8
#define SETPRECHARGE 0xD9
#define SETSEGMENTREMAP 0xA1
#define SETSTARTLINE 0x40
#define SETVCOMDETECT 0xDB
#define SWITCHCAPVCC 0x2

class SSD1306 {

private:
   int myI2cAddress;
   int mySda;
   int mySdc;
   uint8_t buffer[128 * 64 / 8];
   int myTextAlignment = TEXT_ALIGN_LEFT;
   int myColor = WHITE;
   byte lastChar;
   const char *myFontData = ArialMT_Plain_10;

public:
   // Create the display object connected to pin sda and sdc
   SSD1306(int i2cAddress, int sda, int sdc);

   // Initialize the display
   void init();

   // Initialize the display - Neil's version to setting of address
   void init(int i2cAddress);		//local override to allow init to set the address

   // Cycle through the initialization
   void resetDisplay(void);

   // Connect again to the display through I2C
   void reconnect(void);

   // Turn the display on
   void displayOn(void);

   // Turn the display offs
   void displayOff(void);

   // Clear the local pixel buffer
   void clear(void);

   // Write the buffer to the display memory
   void display(void);

   // Set display contrast
   void setContrast(char contrast);

   // Turn the display upside down
   void flipScreenVertically();

   // Send a command to the display (low level function)
   void sendCommand(unsigned char com);

   // Send all the init commands
   void sendInitCommands(void);

   // Draw a pixel at given position
   void setPixel(int x, int y);

   // Draw 8 bits at the given position
   void setChar(int x, int y, unsigned char data);

   // Draw the border of a rectangle at the given location
   void drawRect(int x, int y, int width, int height);

   // Fill the rectangle
   void fillRect(int x, int y, int width, int height);

   // Draw a bitmap with the given dimensions
   void drawBitmap(int x, int y, int width, int height, const char *bitmap);

   // Draw an XBM image with the given dimensions
   void drawXbm(int x, int y, int width, int height, const char *xbm);

   // Sets the color of all pixel operations
   void setColor(int color);

   // converts utf8 characters to extended ascii
   // taken from http://playground.arduino.cc/Main/Utf8ascii
   byte utf8ascii(byte ascii);

   // converts utf8 string to extended ascii
   // taken from http://playground.arduino.cc/Main/Utf8ascii
   String utf8ascii(String s);

   // Draws a string at the given location
   void drawString(int x, int y, String text);

   // Draws a String with a maximum width at the given location.
   // If the given String is wider than the specified width
   // The text will be wrapped to the next line at a space or dash
   void drawStringMaxWidth(int x, int y, int maxLineWidth, String text);

   // Returns the width of the String with the current
   // font settings
   int getStringWidth(String text);

   // Specifies relative to which anchor point
   // the text is rendered. Available constants:
   // TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT
   void setTextAlignment(int textAlignment);

   // Sets the current font. Available default fonts
   // defined in SSD1306Fonts.h:
   // ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
   void setFont(const char *fontData);

};