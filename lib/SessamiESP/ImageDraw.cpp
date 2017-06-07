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

#include "ImageDraw.h"    // Header library

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

void ImageDraw::ClearLCD() {
	tft->fillScreen(ILI9341_BLACK);
}

void ImageDraw::initLCD() {
	Serial.println("---------LCD initialization Start-----------");

	SPIFFS.begin();
	delay(500);
	tft->begin();
	delay(500);
	ClearLCD();
	tft->setRotation(3);

	Serial.println("---------LCD initialization End-----------");
	Serial.println();
}

void ImageDraw::bmpDraw(char *filename, uint8_t x, uint16_t y) {
	File bmpFile;
	int bmpWidth, bmpHeight;   // W+H in pixels
	uint8_t bmpDepth;              // Bit depth (currently must be 24)
	uint32_t bmpImageoffset;        // Start of image data in file
	uint32_t rowSize;               // Not always = bmpWidth; may have padding
	uint8_t sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
	uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
	boolean goodBmp = false;       // Set to true on valid header parse
	boolean flip = true;        // BMP is stored bottom-to-top
	int w, h, row, col;
	uint8_t r, g, b;
	uint32_t pos = 0, startTime = millis();

	if ((x >= tft->width()) || (y >= tft->height()))
		return;

	Serial.println();
	Serial.print(F("Loading image '"));
	Serial.print(filename);
	Serial.println('\'');

	bmpFile = SPIFFS.open(filename, "r");

	if (bmpFile == NULL) {
		Serial.println(F("File not found"));
		return;
	} else
		Serial.println(F("File found....Displaying!"));

	// Parse BMP header
	if (read16(bmpFile) == 0x4D42) { // BMP signature
		Serial.print(F("File size: "));
		Serial.println(read32(bmpFile));

		(void) read32(bmpFile); // Read & ignore creator bytes
		bmpImageoffset = read32(bmpFile); // Start of image data
		Serial.print(F("Image Offset: "));
		Serial.println(bmpImageoffset, DEC);

		// Read DIB header
		Serial.print(F("Header size: "));
		Serial.println(read32(bmpFile));

		bmpWidth = read32(bmpFile);
		bmpHeight = read32(bmpFile);
		if (read16(bmpFile) == 1) { // # planes -- must be '1'
			bmpDepth = read16(bmpFile); // bits per pixel
			Serial.print(F("Bit Depth: "));
			Serial.println(bmpDepth);

			if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
				goodBmp = true; // Supported BMP format -- proceed!
				Serial.print(F("Image size: "));
				Serial.print(bmpWidth);

				Serial.print('x');
				Serial.println(bmpHeight);

				// BMP rows are padded (if needed) to 4-byte boundary
				rowSize = (bmpWidth * 3 + 3) & ~3;

				// If bmpHeight is negative, image is in top-down order.
				// This is not canon but has been observed in the wild.
				if (bmpHeight < 0) {
					bmpHeight = -bmpHeight;
					flip = false;
				}

				// Crop area to be loaded
				w = bmpWidth;
				h = bmpHeight;
				if ((x + w - 1) >= tft->width())
					w = tft->width() - x;
				if ((y + h - 1) >= tft->height())
					h = tft->height() - y;

				// Set TFT address window to clipped image bounds
				tft->setAddrWindow(x, y, x + w - 1, y + h - 1);

				for (row = 0; row < h; row++) { // For each scanline...
												// Seek to start of scan line.  It might seem labor-intensive to be doing this on every line, but this
												// method covers a lot of gritty details like cropping and scanline padding.  Also, the seek only takes
												// place if the file position actually needs to change (avoids a lot of cluster math in SD library).
					if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
						pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
					else
						// Bitmap is stored top-to-bottom
						pos = bmpImageoffset + row * rowSize;
					/*if mode is SeekSet, position is set to offset bytes from the beginning.
					 if mode is SeekCur, current position is moved by offset bytes.
					 if mode is SeekEnd, position is set to offset bytes from the end of the file.*/
					if (bmpFile.position() != pos) { // Need seek?
						bmpFile.seek(pos, SeekSet);
						buffidx = sizeof(sdbuffer); // Force buffer reload
					}

					for (col = 0; col < w; col++) { // For each pixel...
													// Time to read more pixel data?
						if (buffidx >= sizeof(sdbuffer)) { // Indeed
							bmpFile.read(sdbuffer, sizeof(sdbuffer));
							buffidx = 0; // Set index to beginning
						}
						// Convert pixel from BMP to TFT format, push to display
						b = sdbuffer[buffidx++];
						g = sdbuffer[buffidx++];
						r = sdbuffer[buffidx++];
						tft->pushColor(tft->color565(r, g, b));
					} // end pixel
				} // end scanline
				Serial.print(F("Loaded in "));
				Serial.print(millis() - startTime);
				Serial.println(" ms");
			} // end goodBmp
		}
	}
	bmpFile.close();
	if (!goodBmp)
		Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t ImageDraw::read16(File &f) {
	uint16_t result;
	((uint8_t *) &result)[0] = f.read(); // LSB
	((uint8_t *) &result)[1] = f.read(); // MSB
	return result;
}

uint32_t ImageDraw::read32(File &f) {
	uint32_t result;
	((uint8_t *) &result)[0] = f.read(); // LSB
	((uint8_t *) &result)[1] = f.read();
	((uint8_t *) &result)[2] = f.read();
	((uint8_t *) &result)[3] = f.read(); // MSB
	return result;
}

ImageDraw::ImageDraw(Adafruit_ILI9341 &_tft) :
		tft(&_tft) {
}

ImageDraw::~ImageDraw() {
}
