#include "LOLIN_EPD.h"
#include "LOLIN_UC8151D.h"

#define ALLSCREEN_GRAGHBYTES 4000

///////////////////////////////////////////////////

/////////////////EPD settings Functions/////////////////////

#define BUSY_WAIT 500

/*!
    @brief constructor if using on-chip RAM and software SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param SID the SID pin to use
    @param SCLK the SCLK pin to use
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
LOLIN_UC8151D::LOLIN_UC8151D(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, SID, SCLK, DC, RST, CS, BUSY)
{

    if ((height % 8) > 0)
    {
        _height_8bit = (height / 8 + 1) * 8;
    }
    else
    {
        _height_8bit = height;
    }

    bw_buf = (uint8_t *)malloc(width * _height_8bit / 8);
    red_buf = (uint8_t *)malloc(width * _height_8bit / 8);
    bw_bufsize = width * _height_8bit / 8;
    red_bufsize = bw_bufsize;
}

LOLIN_UC8151D::LOLIN_UC8151D(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, DC, RST, CS, BUSY)
{

    if ((height % 8) > 0)
    {
        _height_8bit = (height / 8 + 1) * 8;
    }
    else
    {
        _height_8bit = height;
    }

    bw_buf = (uint8_t *)malloc(width * _height_8bit / 8);
    red_buf = (uint8_t *)malloc(width * _height_8bit / 8);
    bw_bufsize = width * _height_8bit / 8;
    red_bufsize = bw_bufsize;
}

/**************************************************************************/
/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
*/
/**************************************************************************/
void LOLIN_UC8151D::begin(bool reset)
{
    // uint8_t HRES = 0x68;       //104
    // uint8_t VRES_byte1 = 0x00; //212
    // uint8_t VRES_byte2 = 0xd4;

    // uint8_t buf[5];
    LOLIN_EPD::begin(reset);

    sendCmd(0x04);
    readBusy(); //waiting for the electronic paper IC to release the idle signal

    sendCmd(0x00);  //panel setting
    sendData(0x0f); //LUT from OTP£¬128x296
    sendData(0x89); //Temperature sensor, boost and other related timing settings

    sendCmd(0x61); //resolution setting
    sendData(HEIGHT);
    sendData((WIDTH >> 8) & 0xff);
    sendData(WIDTH & 0xff);

    sendCmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    sendData(0x77); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}

void LOLIN_UC8151D::display()
{

    sendCmd(0x10); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        sendData(bw_buf[i]);
    }

    sendCmd(0x13); //write RAM for red(0)/white (1)

    for (uint16_t i = 0; i < red_bufsize; i++)
    {
        sendData(red_buf[i]);
    }

    update();
}
void LOLIN_UC8151D::update()
{
    sendCmd(0x12);
    delay(100);
    readBusy();
}

/**************************************************************************/
/*!
    @brief draw a single pixel on the screen
	@param x the x axis position
	@param y the y axis position
	@param color the color of the pixel
*/
/**************************************************************************/
void LOLIN_UC8151D::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
        return;

    uint8_t *pBuf;

    // check rotation, move pixel around if necessary
    switch (getRotation())
    {
    case 0:
        x = WIDTH - x - 1;
        break;
    case 1:
        EPD_swap(x, y);
        break;
    case 2:
        y = HEIGHT - y - 1;
        break;
    case 3:
        EPD_swap(x, y);
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
    }
    //make our buffer happy
    x = (x == 0 ? 1 : x);

    // uint16_t addr = (x * height() + y) / 8;
    uint16_t addr = (x * _height_8bit + y) / 8;

    if (color == EPD_RED)
    {
        pBuf = red_buf + addr;
    }
    else
    {
        pBuf = bw_buf + addr;
    }

    // x is which column
    switch (color)
    {
    case EPD_WHITE:
        *pBuf |= (1 << (7 - y % 8));
        break;
    case EPD_RED:
    case EPD_BLACK:
        *pBuf &= ~(1 << (7 - y % 8));
        break;
    case EPD_INVERSE:
        *pBuf ^= (1 << (7 - y % 8));
        break;
    }
}

/**************************************************************************/
/*!
    @brief clear all data buffers
*/
/**************************************************************************/
void LOLIN_UC8151D::clearBuffer()
{
    memset(bw_buf, 0xFF, bw_bufsize);
    memset(red_buf, 0xFF, red_bufsize);
}

/**************************************************************************/
/*!
    @brief clear the display twice to remove any spooky ghost images
*/
/**************************************************************************/
void LOLIN_UC8151D::clearDisplay()
{
    clearBuffer();
    display();
    delay(100);
    display();
}

void LOLIN_UC8151D::readBusy()
{
    if (busy >= 0)
    {
        while (1)
        {
            if (digitalRead(busy) == 0)
                break;
        }
    }
    else
    {
        delay(BUSY_WAIT);
    }
}

void LOLIN_UC8151D::deepSleep()
{
    sendCmd(0X50); //VCOM AND DATA INTERVAL SETTING
    sendData(0xf7);

    sendCmd(0X02); //power off
    readBusy();

    sendCmd(0X07); //deep sleep
    sendData(0xA5);
}

void LOLIN_UC8151D::fillbuffer(const unsigned char *black_image, const unsigned char *red_image)
{
    for (int i = 0; i < bw_bufsize; i++)
    {
        bw_buf[i] = pgm_read_byte(&black_image[i]);
    }

    for (int i = 0; i < red_bufsize; i++)
    {
        red_buf[i] = pgm_read_byte(&red_image[i]);
    }
}
