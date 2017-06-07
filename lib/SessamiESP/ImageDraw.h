/**************************************************************************************
 Original Contributor Information
 This is our Bitmap drawing example for the Adafruit ILI9341 Breakout and Shield
 ----> http://www.adafruit.com/products/1651

 Check out the links above for our tutorials and wiring diagrams
 These displays use SPI to communicate, 4 or 5 pins are required to
 interface (RST is optional)
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 MIT license, all text above must be included in any redistribution
 ***************************************************************************************/
/**************************************************************************************
 Title : Display 24-bit Bitmap Image from 4MB ESP-12E Flash on TFT LCD using ILI9341 driver
 Modified by : Sagar Naikwadi, Kaz Wong
 Date : 7 Dec 2016
 Description : Works with the 24-bit BMP image to display on TFT LCD screen connected to ESP
 ***************************************************************************************/

#ifndef IMAGEDRAW_H_
#define IMAGEDRAW_H_

#include "Adafruit_GFX.h"    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include <SPI.h>
#include "FS.h"

#define BUFFPIXEL 20

class ImageDraw {
private:
	Adafruit_ILI9341 *tft;

public:
	ImageDraw(Adafruit_ILI9341 &_tft);
	~ImageDraw();
	void initLCD();
	void ClearLCD();
	void bmpDraw(char *, uint8_t, uint16_t);
	uint16_t read16(File &);
	uint32_t read32(File &f);

};

#endif
