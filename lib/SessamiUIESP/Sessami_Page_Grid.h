/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : Class of Sessami UI page to display grid
 ***************************************************************************************/

#include "SessamiUI.h"

class Page_Grid : private SessamiUI {
  public:
    virtual uint8_t UIStateMachine(bool rst);
    uint8_t C0(unsigned int color);

    Page_Grid();
    virtual ~Page_Grid();
};

Page_Grid::Page_Grid() {
}

Page_Grid::~Page_Grid() {

}

uint8_t Page_Grid::UIStateMachine(bool rst) {
  if (rst)
    C0(ILI9341_WHITE);
  return 0;
}

//Draw grid
uint8_t Page_Grid::C0(unsigned int color) {
  for (int i=0; i<SCREENHEIGHT; i+=10) {
    tft.drawFastHLine(0, i, SCREENWIDTH, color); //Horizontal
  }
  
  for (int i=0; i<SCREENWIDTH; i+=10) {
    tft.drawFastVLine(i, 0, SCREENHEIGHT, color); //Vertical
  }

   tft.drawFastHLine(0, 120, SCREENWIDTH,  ILI9341_RED); //Horizontal mid
   tft.drawFastVLine(160, 0, SCREENHEIGHT,  ILI9341_RED); //Vertical mid
}
