#include "LOLIN_EPD.h"
#include "LOLIN_SSD1680.h"

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
LOLIN_SSD1680::LOLIN_SSD1680(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, SID, SCLK, DC, RST, CS, BUSY)
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

LOLIN_SSD1680::LOLIN_SSD1680(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, DC, RST, CS, BUSY)
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
void LOLIN_SSD1680::begin(bool reset)
{
    // uint8_t HRES = 0x68;       //104
    // uint8_t VRES_byte1 = 0x00; //212
    // uint8_t VRES_byte2 = 0xd4;

    // uint8_t buf[5];
    LOLIN_EPD::begin(reset);

    readBusy();
    sendCmd(0x12); //SWRESET
    readBusy();

    sendCmd(0x01); //Driver output control
    sendData(0xF9);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x11); //data entry mode
    sendData(0x01);

    sendCmd(0x44); //set Ram-X address start/end position
    sendData(0x00);
    sendData(0x0F); //0x0F-->(15+1)*8=128

    sendCmd(0x45);  //set Ram-Y address start/end position
    sendData(0xF9); //0xF9-->(249+1)=250
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x3C); //BorderWavefrom
    sendData(0x05);

    sendCmd(0x18); //Read built-in temperature sensor
    sendData(0x80);

    sendCmd(0x21); //  Display update control
    sendData(0x00);
    sendData(0x80);

    sendCmd(0x4E); // set RAM x address count to 0;
    sendData(0x00);
    sendCmd(0x4F); // set RAM y address count to 0X199;
    sendData(0xF9);
    sendData(0x00);
    readBusy();

    // sendData(HEIGHT);
    // sendData((WIDTH >> 8) & 0xff);
    // sendData(WIDTH & 0xff);
}

void LOLIN_SSD1680::display()
{

    sendCmd(0x24); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        sendData(bw_buf[i]);
    }

    sendCmd(0x26); //write RAM for red(1)/white (0)

    for (uint16_t i = 0; i < red_bufsize; i++)
    {
        sendData(red_buf[i]);
    }

    update();
}
void LOLIN_SSD1680::update()
{
    sendCmd(0x22); //Display Update Control
    sendData(0xF7);
    sendCmd(0x20); //Activate Display Update Sequence
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
void LOLIN_SSD1680::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
        return;

    uint8_t *pBuf;

    // check rotation, move pixel around if necessary
    switch (getRotation())
    {

    case 1:
        EPD_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
    case 3:
        EPD_swap(x, y);
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
        *pBuf |= (1 << (7 - y % 8));
        break;
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
void LOLIN_SSD1680::clearBuffer()
{
    memset(bw_buf, 0xFF, bw_bufsize);
    memset(red_buf, 0x00, red_bufsize);
}

/**************************************************************************/
/*!
    @brief clear the display twice to remove any spooky ghost images
*/
/**************************************************************************/
void LOLIN_SSD1680::clearDisplay()
{
    clearBuffer();
    display();
    delay(100);
    display();
}

void LOLIN_SSD1680::readBusy()
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

void LOLIN_SSD1680::deepSleep()
{

    sendCmd(0x10); //enter deep sleep
    sendData(0x01);
    delay(100);
}

void LOLIN_SSD1680::fillbuffer(const unsigned char *black_image, const unsigned char *red_image)
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
