/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 5 Jan 2017
  Description : Implement and Test Keypad
 ***************************************************************************************/

#include "SessamiUI.h"

class Page_KeypadTest : private SessamiUI {
  private: 
    char idx;
  public:
    virtual uint8_t UIStateMachine(bool rst);
    uint8_t C0(unsigned int color);
    uint8_t C1(unsigned int color);

    Page_KeypadTest();
    virtual ~Page_KeypadTest();
};

Page_KeypadTest::Page_KeypadTest() : idx('a') {
}

Page_KeypadTest::~Page_KeypadTest() {

}

uint8_t Page_KeypadTest::UIStateMachine(bool rst) {
  if (rst) {
    image_draw->ClearLCD();
    state = 0;
  }
  switch (state) {
    case 0 : C0(ILI9341_WHITE);
    case 1 :
      C1(ILI9341_WHITE);
      state = 2;
      break;
  }
  
  if (*button == S_RIGHT) {
    C1(ILI9341_BLACK);
    idx++;
    C1(ILI9341_WHITE);
  } else if (*button == S_LEFT) {
    C1(ILI9341_BLACK);
    idx--;
    C1(ILI9341_WHITE);
  }
  return 0;
}

uint8_t Page_KeypadTest::C0(unsigned int color) {
  tft.setTextColor(color);
  tft.setFont(&LiberationSans_Regular48pt7b);
  tft.setCursor(45, 80);
  tft.print("Text : ");
  
  tft.drawRect(25, 120, 270, 50, color);
  
  return 0;
}

uint8_t Page_KeypadTest::C1(unsigned int color) {
  tft.setTextColor(color);
  tft.setFont(&LiberationSans_Regular16pt7b);

  tft.setCursor(15, 225);
  tft.print((char)idx);

  tft.setCursor(55, 225);
  tft.print((char)(idx+1));

  tft.setCursor(95, 225);
  tft.print((char)(idx+2));

  tft.setCursor(135, 225);
  tft.print((char)(idx+3));

  tft.setCursor(175, 225);
  tft.print((char)(idx+4));
  
  tft.setCursor(215, 225);
  tft.print((char)(idx+5));

  tft.setCursor(255, 225);
  tft.print((char)(idx+6));

  tft.setCursor(295, 225);
  tft.print((char)(idx+7));

  tft.drawRect(10, 200, 30, 30, color);
  return 0;
}
