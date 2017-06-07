/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : Class of Sessami UI page to display grid
 ***************************************************************************************/

#include "SessamiUI.h"

class Page_Blank : private SessamiUI {
  public:
    virtual uint8_t UIStateMachine(bool rst);
    uint8_t C0(unsigned int color);

    Page_Blank();
    virtual ~Page_Blank();
};

Page_Blank::Page_Blank() {
}

Page_Blank::~Page_Blank() {

}

uint8_t Page_Blank::UIStateMachine(bool rst) {
  if (rst)
    C0(ILI9341_WHITE);
  return 0;
}

//Draw grid
uint8_t Page_Blank::C0(unsigned int color) {
  image_draw->ClearLCD();
}
