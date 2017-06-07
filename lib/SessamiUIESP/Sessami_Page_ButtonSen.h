/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 5 Jan 2017
  Description : Test Button Sensitivity (Data Sensitivity, Delta count and Threshold) for implementation
 ***************************************************************************************/

#include "SessamiUI.h"

extern volatile unsigned long int_t;
extern volatile unsigned long rp_int;

class Page_ButtonSen : private SessamiUI {
  private:
    int MAX_ITEM;
    unsigned long loop_t;
    unsigned long last_t;
    int idx;
    bool prt;
    bool lock;
    int8_t last_delta_count[7];
    int8_t delta_count[7];
  public:
    virtual uint8_t UIStateMachine(bool rst);
    uint8_t C0(unsigned int color);
    uint8_t C1(unsigned int color);
    uint8_t C2(unsigned int color);
    uint8_t Menu(bool prt);
    uint8_t C4(unsigned int i);
    uint8_t C5(unsigned int color);

    Page_ButtonSen();
    virtual ~Page_ButtonSen();
};

Page_ButtonSen::Page_ButtonSen() : idx(0), prt(true), lock(false), loop_t(0), last_t(0), MAX_ITEM(9) {
}

Page_ButtonSen::~Page_ButtonSen() {

}

uint8_t Page_ButtonSen::UIStateMachine(bool rst) {
  if (rst) {
    image_draw->ClearLCD();
    C0(ILI9341_WHITE);
    C1(ILI9341_WHITE);
    
    prt = true;
    //state = 0;
  }
  C2(ILI9341_WHITE);
  
  Menu(prt);
  prt = false;

  C5(ILI9341_WHITE);
  return 0;
}

uint8_t Page_ButtonSen::C0(unsigned int color) {
  tft.setTextColor(color);

  tft.setFont(&LiberationSans_Regular6pt7b);
  tft.setCursor(250, 10);
  tft.print("Sens");

  return 0;
}

uint8_t Page_ButtonSen::C1(unsigned int color) {
  tft.fillRect(20, 236, 60, 4, ILI9341_WHITE);
  tft.fillRect(130, 236, 60, 4, ILI9341_WHITE);
  tft.fillRect(240, 236, 60, 4, ILI9341_WHITE);
}

uint8_t Page_ButtonSen::C2(unsigned int color) {
  tft.setFont(&LiberationSans_Regular6pt7b);
  
  tft.setCursor(285, 10);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(loop_t);

  tft.setCursor(285, 10);
  tft.setTextColor(color);
  loop_t = millis() - last_t;
  tft.print(loop_t);
  last_t = millis();
}

uint8_t Page_ButtonSen::C4(unsigned int i) {
  tft.setFont(&LiberationSans_Regular10pt7b);

  if (idx == i) {
    tft.fillRect(10, 20 + i * 20, 310, 18, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
  } else {
    tft.fillRect(10, 20 + i * 20, 310, 18, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
  }

  switch (i) {
    case 0 :
      tft.setCursor(10, 35);
      tft.print("Prox");
      
      tft.setCursor(220, 35);
      tft.print(button->GetTh(B_PROX));
      break;
    case 1 :
      tft.setCursor(10, 55);
      tft.print("Up");
      
      tft.setCursor(220, 55);
      tft.print(button->GetTh(B_UP));
      break;
    case 2 :
      tft.setCursor(10, 75);
      tft.print("Down");
      
      tft.setCursor(220, 75);
      tft.print(button->GetTh(B_DOWN));
      break;
    case 3 :
      tft.setCursor(10, 95);
      tft.print("Power");
      
      tft.setCursor(220, 95);
      tft.print(button->GetTh(B_POWER));
      break;
    case 4 :
      tft.setCursor(10, 115);
      tft.print("Right");
      
      tft.setCursor(220, 115);
      tft.print(button->GetTh(B_RIGHT));
      break;
    case 5 :
      tft.setCursor(10, 135);
      tft.print("Mid");
      
      tft.setCursor(220, 135);
      tft.print(button->GetTh(B_MID));
      break;
    case 6 :
      tft.setCursor(10, 155);
      tft.print("Left");
      
      tft.setCursor(220, 155);
      tft.print(button->GetTh(B_LEFT));
      break;
    case 7 :
      tft.setCursor(10, 175);
      tft.print("Data Sen");
      tft.setCursor(220, 175);
      tft.print(button->GetButSen());
      break;
    case 8 :
      tft.setCursor(10, 195);
      tft.print("Prox Sen");
      tft.setCursor(220, 195);
      tft.print(button->GetPROXSen());
      break;
  }
}

uint8_t Page_ButtonSen::C5(unsigned int color) {
  tft.setFont(&LiberationSans_Regular10pt7b);

  for (int i=0; i<7; i++) {
    if (idx == i) {
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(150, 35 + i * 20);
      tft.print(last_delta_count[i]);
      tft.setTextColor(ILI9341_BLACK);
    } else {
      tft.setTextColor(ILI9341_BLACK);
      tft.setCursor(150, 35 + i * 20);
      tft.print(last_delta_count[i]);
      tft.setTextColor(ILI9341_WHITE);
    }
    last_delta_count[i] = delta_count[i];
    switch (i) {
    case 0 :
      delta_count[i] = button->GetDeltaCount(B_PROX);
      break;
    case 1 :
      delta_count[i] = button->GetDeltaCount(B_UP);
      break;
    case 2 :
      delta_count[i] = button->GetDeltaCount(B_DOWN);
      break;
    case 3 :
      delta_count[i] = button->GetDeltaCount(B_POWER);
      break;
    case 4 :
      delta_count[i] = button->GetDeltaCount(B_RIGHT);
      break;
    case 5 :
      delta_count[i] = button->GetDeltaCount(B_MID);
      break;
    case 6 :
      delta_count[i] = button->GetDeltaCount(B_LEFT);
      break;
  }
    tft.setCursor(150, 35 + i * 20);
    tft.print(delta_count[i]);
  }
}

uint8_t Page_ButtonSen::Menu(bool prt) {
  if (prt) {
    for (int i=0; i<MAX_ITEM; i++)
      C4(i);
  }
  
  if (*button == B_UP)  {
    int tmp = idx;
    if (--idx < 0)
      idx = MAX_ITEM - 1;
    C4(tmp);
    C4(idx);
  } else if (*button == B_DOWN) {
    int tmp = idx;
    if (++idx > MAX_ITEM - 1)
      idx = 0;
    C4(tmp);
    C4(idx);
  }

  int value = 0;
  switch (idx) {
    case 0:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_PROX, button->Getthreshold(B_PROX) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_PROX, button->Getthreshold(B_PROX) + 1);
        C4(idx);
      }
      break;
    case 1:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_UP, button->Getthreshold(B_UP) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_UP, button->Getthreshold(B_UP) + 1);
        C4(idx);
      }
      break;
    case 2:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_DOWN, button->Getthreshold(B_DOWN) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_DOWN, button->Getthreshold(B_DOWN) + 1);
        C4(idx);
      }
      break;
    case 3:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_POWER, button->Getthreshold(B_POWER) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_POWER, button->Getthreshold(B_POWER) + 1);
        C4(idx);
      }
      break;
    case 4:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_RIGHT, button->Getthreshold(B_RIGHT) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_RIGHT, button->Getthreshold(B_RIGHT) + 1);
        C4(idx);
      }
      break;
    case 5:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_MID, button->Getthreshold(B_MID) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_MID, button->Getthreshold(B_MID) + 1);
        C4(idx);
      }
      break;
    case 6:
      if (*button == S_LEFT) {//( (*button == S_LEFT) || (*button == S_RIGHT) ) {
        button->SetTh(B_LEFT, button->Getthreshold(B_LEFT) - 1);
        C4(idx);
      } else if (*button == S_RIGHT) {
        button->SetTh(B_LEFT, button->Getthreshold(B_LEFT) + 1);
        C4(idx);
      }
      break;
    case 7:
      value = button->GetButSen();
      if ( (*button == S_LEFT) && (value > 0) ) {
        value--;
        C4(idx);
        button->SetButSen(value);
      } else if ( (*button == S_RIGHT) && (value < 7) ) {
        value++;
        C4(idx);
        button->SetButSen(value);
      }
      break;
    case 8:
      value = button->GetPROXSen();
      if ( (*button == S_LEFT) && (value > 0) ) {
        value--;
        C4(idx);
        button->SetPROXSen(value);
      } else if ( (*button == S_RIGHT) && (value < 7) ) {
        value++;
        C4(idx);
        button->SetPROXSen(value);
      }
      break;
  }
}
