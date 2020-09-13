#include <LOLIN_EPD.h>
#include <Adafruit_GFX.h>

float p = 3.1415926;

/*D1 mini*/
#define EPD_CS D0
#define EPD_DC D8
#define EPD_RST -1  // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)

/*D32 Pro*/
// #define EPD_CS 14
// #define EPD_DC 27
// #define EPD_RST 33  // can set to -1 and share with microcontroller Reset!
// #define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)

LOLIN_IL3897 EPD(250, 122, EPD_DC, EPD_RST, EPD_CS, EPD_BUSY); //hardware SPI

// #define EPD_MOSI D7
// #define EPD_CLK D5
// LOLIN_IL3897 EPD(250,122, EPD_MOSI, EPD_CLK, EPD_DC, EPD_RST, EPD_CS, EPD_BUSY); //IO



void setup(void) {
  Serial.begin(115200);
  Serial.print("Hello! EPD Test");

  EPD.begin();

  Serial.println("Initialized");

  // large block of text
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", EPD_BLACK);

  // epd print function!
  epdPrintTest();

  // a single pixel
  EPD.clearBuffer();
  EPD.drawPixel(EPD.width()/2, EPD.height()/2, EPD_BLACK);

  testtriangles();
  
  // line draw test
  testlines(EPD_BLACK);

  // optimized lines
  testfastlines(EPD_BLACK, EPD_RED);

  testdrawrects(EPD_RED);

  testfillrects(EPD_BLACK, EPD_RED);

  EPD.fillScreen(EPD_WHITE);
  testfillcircles(10, EPD_RED);
  testdrawcircles(10, EPD_BLACK);

  testroundrects();

  mediabuttons();

  Serial.println("done");
}

void loop() {
  delay(500);
}

void testlines(uint16_t color) {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=0; x < EPD.width(); x+=6) {
    EPD.drawLine(0, 0, x, EPD.height()-1, color);
  }
  for (int16_t y=0; y < EPD.height(); y+=6) {
    EPD.drawLine(0, 0, EPD.width()-1, y, color);
  }

  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=0; x < EPD.width(); x+=6) {
    EPD.drawLine(EPD.width()-1, 0, x, EPD.height()-1, color);
  }
  for (int16_t y=0; y < EPD.height(); y+=6) {
    EPD.drawLine(EPD.width()-1, 0, 0, y, color);
  }

  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=0; x < EPD.width(); x+=6) {
    EPD.drawLine(0, EPD.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < EPD.height(); y+=6) {
    EPD.drawLine(0, EPD.height()-1, EPD.width()-1, y, color);
  }

  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=0; x < EPD.width(); x+=6) {
    EPD.drawLine(EPD.width()-1, EPD.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < EPD.height(); y+=6) {
    EPD.drawLine(EPD.width()-1, EPD.height()-1, 0, y, color);
  }
  EPD.display();
}

void testdrawtext(char *text, uint16_t color) {
  EPD.clearBuffer();
  EPD.setCursor(0, 0);
  EPD.setTextColor(color);
  EPD.setTextWrap(true);
  EPD.print(text);
  EPD.display();
}

void testfastlines(uint16_t color1, uint16_t color2) {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  for (int16_t y=0; y < EPD.height(); y+=5) {
    EPD.drawFastHLine(0, y, EPD.width(), color1);
  }
  for (int16_t x=0; x < EPD.width(); x+=5) {
    EPD.drawFastVLine(x, 0, EPD.height(), color2);
  }
  EPD.display();
}

void testdrawrects(uint16_t color) {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=0; x < EPD.width(); x+=6) {
    EPD.drawRect(EPD.width()/2 -x/2, EPD.height()/2 -x/2 , x, x, color);
  }
  EPD.display();
}

void testfillrects(uint16_t color1, uint16_t color2) {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  for (int16_t x=EPD.width()-1; x > 6; x-=6) {
    EPD.fillRect(EPD.width()/2 -x/2, EPD.height()/2 -x/2 , x, x, color1);
    EPD.drawRect(EPD.width()/2 -x/2, EPD.height()/2 -x/2 , x, x, color2);
  }
  EPD.display();
}

void testfillcircles(uint8_t radius, uint16_t color) {
  EPD.clearBuffer();
  for (int16_t x=radius; x < EPD.width(); x+=radius*2) {
    for (int16_t y=radius; y < EPD.height(); y+=radius*2) {
      EPD.fillCircle(x, y, radius, color);
    }
  }
  EPD.display();
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  EPD.clearBuffer();
  for (int16_t x=0; x < EPD.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < EPD.height()+radius; y+=radius*2) {
      EPD.drawCircle(x, y, radius, color);
    }
  }
  EPD.display();
}

void testtriangles() {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  int color = EPD_BLACK;
  int t;
  int w = EPD.width()/2;
  int x = EPD.height()-1;
  int y = 0;
  int z = EPD.width();
  for(t = 0 ; t <= 15; t++) {
    EPD.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    if(t == 8) color = EPD_RED;
  }
  EPD.display();
}

void testroundrects() {
  EPD.clearBuffer();
  EPD.fillScreen(EPD_WHITE);
  int color = EPD_BLACK;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = EPD.width()-2;
    int h = EPD.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      EPD.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      if(i == 7) color = EPD_RED;
    }
    color = EPD_BLACK;
  }
  EPD.display();
}

void epdPrintTest() {
  EPD.clearBuffer();
  EPD.setCursor(2, 0);
  EPD.fillScreen(EPD_WHITE);
  EPD.setTextColor(EPD_BLACK);
  EPD.setTextSize(2);
  EPD.println("Hello World!");
  EPD.setTextSize(1);
  EPD.setTextColor(EPD_RED);
  EPD.print(p, 6);
  EPD.println(" Want pi?");
  EPD.println(" ");
  EPD.print(8675309, HEX); // print 8,675,309 out in HEX!
  EPD.println(" Print HEX!");
  EPD.println(" ");
  EPD.setTextColor(EPD_BLACK);
  EPD.println("Sketch has been");
  EPD.println("running for: ");
  EPD.setTextColor(EPD_RED);
  EPD.print(millis() / 1000);
  EPD.setTextColor(EPD_BLACK);
  EPD.print(" seconds.");
  EPD.display();
}

void mediabuttons() {
  EPD.clearBuffer();
  // play
  EPD.fillScreen(EPD_WHITE);
  EPD.fillRoundRect(25, 10, 78, 60, 8, EPD_BLACK);
  EPD.fillTriangle(42, 20, 42, 60, 90, 40, EPD_RED);
  // pause
  EPD.fillRoundRect(25, 90, 78, 60, 8, EPD_BLACK);
  EPD.fillRoundRect(39, 98, 20, 45, 5, EPD_RED);
  EPD.fillRoundRect(69, 98, 20, 45, 5, EPD_RED);
  EPD.display();
}
