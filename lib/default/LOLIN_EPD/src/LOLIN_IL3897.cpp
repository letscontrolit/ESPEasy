#include "LOLIN_EPD.h"
#include "LOLIN_IL3897.h"

#define ALLSCREEN_GRAGHBYTES 4000

///////////////////////////////////////////////////


/////////////////EPD settings Functions/////////////////////

/////////////////////////////////////LUT//////////////////////////////////////////////
const unsigned char LUT_DATA[] PROGMEM = {
    0x80,
    0x60,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00, //LUT0: BB:     VS 0 ~7
    0x10,
    0x60,
    0x20,
    0x00,
    0x00,
    0x00,
    0x00, //LUT1: BW:     VS 0 ~7
    0x80,
    0x60,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00, //LUT2: WB:     VS 0 ~7
    0x10,
    0x60,
    0x20,
    0x00,
    0x00,
    0x00,
    0x00, //LUT3: WW:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT4: VCOM:   VS 0 ~7

    0x03,
    0x03,
    0x00,
    0x00,
    0x02, // TP0 A~D RP0
    0x09,
    0x09,
    0x00,
    0x00,
    0x02, // TP1 A~D RP1
    0x03,
    0x03,
    0x00,
    0x00,
    0x02, // TP2 A~D RP2
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP3 A~D RP3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP4 A~D RP4
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP5 A~D RP5
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP6 A~D RP6

    0x15,
    0x41,
    0xA8,
    0x32,
    0x30,
    0x0A,
};
const unsigned char LUT_DATA_part[] PROGMEM = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT0: BB:     VS 0 ~7
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT1: BW:     VS 0 ~7
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT2: WB:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT3: WW:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT4: VCOM:   VS 0 ~7

    0x0A,
    0x00,
    0x00,
    0x00,
    0x00, // TP0 A~D RP0
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP1 A~D RP1
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP2 A~D RP2
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP3 A~D RP3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP4 A~D RP4
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP5 A~D RP5
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP6 A~D RP6

    0x15,
    0x41,
    0xA8,
    0x32,
    0x30,
    0x0A,
};

#define BUSY_WAIT 1000

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
LOLIN_IL3897::LOLIN_IL3897(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, SID, SCLK, DC, RST, CS, BUSY)
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

LOLIN_IL3897::LOLIN_IL3897(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : LOLIN_EPD(width, height, DC, RST, CS, BUSY)
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
void LOLIN_IL3897::begin(bool reset)
{
    LOLIN_EPD::begin(reset);

    readBusy();
    sendCmd(0x12); // soft reset
    readBusy();

    sendCmd(0x74); //set analog block control
    sendData(0x54);
    sendCmd(0x7E); //set digital block control
    sendData(0x3B);

    sendCmd(0x01); //Driver output control
    sendData(0xF9);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x11); //data entry mode
    sendData(0x01);

    sendCmd(0x44); //set Ram-X address start/end position
    sendData(0x00);
    sendData(0x0F); //0x0C-->(15+1)*8=128

    sendCmd(0x45);  //set Ram-Y address start/end position
    sendData(0xF9); //0xF9-->(249+1)=250
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x3C); //BorderWavefrom
    sendData(0x03);

    sendCmd(0x2C);  //VCOM Voltage
    sendData(0x55); //

    sendCmd(0x03); //
    sendData(LUT_DATA[70]);

    sendCmd(0x04); //
    sendData(LUT_DATA[71]);
    sendData(LUT_DATA[72]);
    sendData(LUT_DATA[73]);

    sendCmd(0x3A); //Dummy Line
    sendData(LUT_DATA[74]);
    sendCmd(0x3B); //Gate time
    sendData(LUT_DATA[75]);

    selectLUT((unsigned char *)LUT_DATA); //LUT

    sendCmd(0x4E); // set RAM x address count to 0;
    sendData(0x00);
    sendCmd(0x4F); // set RAM y address count to 0X127;
    sendData(0xF9);
    sendData(0x00);
    readBusy();
}

void LOLIN_IL3897::display()
{

    sendCmd(0x24); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        sendData(bw_buf[i]);
    }

    update();
}
void LOLIN_IL3897::update()
{
    sendCmd(0x22);
    sendData(0xC7);
    sendCmd(0x20);
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
void LOLIN_IL3897::drawPixel(int16_t x, int16_t y, uint16_t color)
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
void LOLIN_IL3897::clearBuffer()
{
    memset(bw_buf, 0xFF, bw_bufsize);
}

/**************************************************************************/
/*!
    @brief clear the display twice to remove any spooky ghost images
*/
/**************************************************************************/
void LOLIN_IL3897::clearDisplay()
{
    clearBuffer();
    display();
    delay(100);
    display();
}

void LOLIN_IL3897::readBusy()
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

void LOLIN_IL3897::selectLUT(uint8_t *wave_data)
{
    uint8_t count;
    sendCmd(0x32);
    for (count = 0; count < 70; count++)
        sendData(pgm_read_byte(&wave_data[count]));
}

void LOLIN_IL3897::deepSleep()
{
    sendCmd(0x10); //enter deep sleep
    sendData(0x01);
    delay(100);
}

void LOLIN_IL3897::partInit()
{
    partBaseImg();

    sendCmd(0x2C); //VCOM Voltage
    sendData(0x26);

    readBusy();
    selectLUT((unsigned char *)LUT_DATA_part);
    sendCmd(0x37);
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);
    sendData(0x40);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x22);
    sendData(0xC0);
    sendCmd(0x20);
    readBusy();

    sendCmd(0x3C); //BorderWavefrom
    sendData(0x01);
}

void LOLIN_IL3897::partBaseImg()
{
    sendCmd(0x24); ////Write Black and White image to RAM

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        sendData(bw_buf[i]);
    }

    sendCmd(0x26); ////Write Black and White image to RAM

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        sendData(bw_buf[i]);
    }

    update();
}

void LOLIN_IL3897::partUpdate()
{
    sendCmd(0x22);
    sendData(0x0C);
    sendCmd(0x20);
    readBusy();
}

void LOLIN_IL3897::partDisplay(int16_t x_start, int16_t y_start, const unsigned char *datas, int16_t PART_COLUMN, int16_t PART_LINE)
{
    int16_t i;
    int16_t x_end, y_start1, y_start2, y_end1, y_end2;
    x_start = x_start / 8; //
    x_end = x_start + PART_LINE / 8 - 1;

    y_start1 = 0;
    y_start2 = y_start;
    if (y_start >= 256)
    {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    y_end1 = 0;
    y_end2 = y_start + PART_COLUMN - 1;
    if (y_end2 >= 256)
    {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }

    sendCmd(0x44);      // set RAM x address start/end, in page 35
    sendData(x_start);  // RAM x address start at 00h;
    sendData(x_end);    // RAM x address end at 0fh(15+1)*8->128
    sendCmd(0x45);      // set RAM y address start/end, in page 35
    sendData(y_start2); // RAM y address start at 0127h;
    sendData(y_start1); // RAM y address start at 0127h;
    sendData(y_end2);   // RAM y address end at 00h;
    sendData(y_end1);   // ????=0

    sendCmd(0x4E); // set RAM x address count to 0;
    sendData(x_start);
    sendCmd(0x4F); // set RAM y address count to 0X127;
    sendData(y_start2);
    sendData(y_start1);

    sendCmd(0x24); //Write Black and White image to RAM
    for (i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
    {
        sendData(pgm_read_byte(&datas[i]));
    }
    partUpdate();
}
