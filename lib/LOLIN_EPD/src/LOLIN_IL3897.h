#ifndef __LOLIN_IL3897_H
#define __LOLIN_IL3897_H

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr)) ///< read bytes from program memory
#endif

#include "LOLIN_EPD.h"


/**************************************************************************/
/*!
    @brief  Class for interfacing with IL3897 EPD drivers
*/
/**************************************************************************/
class LOLIN_IL3897 : public LOLIN_EPD {
	public:

	  LOLIN_IL3897(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY = -1);
	  LOLIN_IL3897(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY = -1);

	void begin(bool reset=true);
	
	void drawPixel(int16_t x, int16_t y, uint16_t color);
	
	void display();
	void update();
	
	void clearBuffer();
	void clearDisplay();

	void deepSleep();

	void partBaseImg();
	void partInit();
	void partDisplay(int16_t x_start, int16_t y_start, const unsigned char *datas, int16_t PART_COLUMN, int16_t PART_LINE);
	void partUpdate();

protected:
	void readBusy();
	void selectLUT(uint8_t * wave_data);
	int _height_8bit;// height 8-bit alignment
};

#endif