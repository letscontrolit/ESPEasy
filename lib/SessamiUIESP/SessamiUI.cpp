/**************************************************************************************
  Title : Sessami UI parent class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : template parent class for all Sessami UI class
 ***************************************************************************************/

#include "SessamiUI.h"

bool SessamiUI::ui_init = false;
Adafruit_ILI9341 SessamiUI::tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
ImageDraw *SessamiUI::image_draw = new ImageDraw(tft);
Sessami_Button *SessamiUI::button;

void SessamiUI::initScr() {
	image_draw->initLCD();
	image_draw->ClearLCD();
}

char* SessamiUI::Keyboard() {
  uint8_t key_x = 0;
  uint8_t key_y = 0;

  image_draw->ClearLCD();
  DrawKeyboard();
}

uint8_t SessamiUI::EvLoop() {
  
}

uint8_t SessamiUI::EvSec() {
  
}

uint8_t SessamiUI::EvMin() {
  
}

uint8_t SessamiUI::EvHr() {
  
}

SessamiUI::SessamiUI() : state(0) {
  if (!ui_init) {
    initScr();
    button = new Sessami_Button;
    ui_init = true;
  }
}

SessamiUI::~SessamiUI() {
  delete image_draw;
}

void SessamiUI::DrawKeyboard() {
  tft.setFont(&LiberationSans_Regular28pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(180, 60);
  tft.print("0");

  tft.setCursor(190, 170);
  tft.print("1");
  
  tft.setCursor(190, 190);
  tft.print("2");
  
  tft.setCursor(190, 210);
  tft.print("3");

  tft.setCursor(190, 230);
  tft.print("4");
}

