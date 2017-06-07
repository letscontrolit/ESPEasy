/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : Class of Sessami UI page
 ***************************************************************************************/

#include "SessamiUI.h"
#include <OpenWeather_ESP8266.h>
#include "Thermostat.h"

extern tmElements_t _time;

class Page_StbNor : private SessamiUI {
  private:
    unsigned long loop_t;
    unsigned long last_t;
    OpenWeather_ESP8266 openweather;
    uint8_t state_count;
    unsigned long anime_t;
    uint8_t _step;
    bool up;
    Coordinates co[6];

    uint8_t PageTitle(unsigned int color);
    uint8_t LoopTime(unsigned int color);
    uint8_t Time(unsigned int color);
    uint8_t Date(unsigned int color);
    uint8_t C0(unsigned int color);
    uint8_t C1(unsigned int color);
    uint8_t C2(unsigned int color);
    uint8_t C3(unsigned int color);


  public:
    virtual uint8_t EvMin();
    virtual uint8_t EvHr();
    virtual uint8_t UIStateMachine(bool rst);

    Page_StbNor();
    virtual ~Page_StbNor();
};

Page_StbNor::Page_StbNor() : up(true) {
  co[0].x = 160; co[0].y = 170;
  co[1].x = 45; co[1].y = 170;
  co[2].x = 50; co[2].y = 85;
  co[3].x = 222; co[3].y = 195;

  state = 0;
  _step = 6;
}

Page_StbNor::~Page_StbNor() {

}

uint8_t Page_StbNor::EvMin() {
  Time(ILI9341_WHITE);
}

uint8_t Page_StbNor::EvHr() {
  Date(ILI9341_WHITE);
}

uint8_t Page_StbNor::UIStateMachine(bool rst) {
  if (rst) {
    //image_draw->ClearLCD();
    PageTitle(ILI9341_WHITE);
    //Time(ILI9341_WHITE);
    //Date(ILI9341_WHITE);
    state_count = 0;
    anime_t = millis();
  }
  while (state_count < 5) {
    if (millis() - anime_t >= 50) {
      LoopTime(ILI9341_WHITE);
      //tft.fillRect(45, 100, 260, 197, ILI9341_BLACK);
      C0(ILI9341_BLACK);
      C1(ILI9341_BLACK);
      C3(ILI9341_BLACK);
      if (up) {
        co[0].y -= _step;
        co[1].y -= _step;
        co[3].y -= _step;
      } else {
        co[0].y += _step;
        co[1].y += _step;
        co[3].y += _step;
      }
      if ((state_count != 0) || (state_count != 4)) {
        C0(ILI9341_WHITE);
        C1(ILI9341_WHITE);
        C3(ILI9341_WHITE);
      }
      /*C0(ILI9341_BLACK);
      if (up)
        co[0].y -= _step;
      else
        co[0].y += _step;
      if ((state_count != 0) || (state_count != 4))
        C0(ILI9341_WHITE);

      C3(ILI9341_BLACK);
      if (up)
        co[3].y -= _step;
      else
        co[3].y += _step;
      if ((state_count != 0) || (state_count != 4))
        C3(ILI9341_WHITE);

      C1(ILI9341_BLACK);
      if (up)
        co[1].y -= _step;
      else
        co[1].y += _step;
      if ((state_count != 0) || (state_count != 4))
        C1(ILI9341_WHITE);*/

      anime_t = millis();
      state_count++;
    }
  }

  up = !up;

  return 0;
}

uint8_t Page_StbNor::PageTitle(unsigned int color) {
  tft.fillRect(235, 0, 100, 13, ILI9341_BLACK);
  tft.setTextColor(color);

  tft.setFont(&LiberationSans_Regular6pt7b);
  tft.setCursor(235, 10);
  tft.print("Stb-Nor");

  return 0;
}

uint8_t Page_StbNor::LoopTime(unsigned int color) {
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

uint8_t Page_StbNor::Time(unsigned int color) {
  tft.fillRect(140, 0, 52, 22, ILI9341_BLACK);
  tft.setTextColor(color);
  tft.setFont(&LiberationSans_Regular10pt7b);
  tft.setCursor(130, 15);
  if (_time.Hour > 9)
    tft.print(_time.Hour);
  else {
    tft.print("0");
    tft.print(_time.Hour);
  }
  tft.print(":");
  if (_time.Minute > 9)
    tft.print(_time.Minute);
  else {
    tft.print("0");
    tft.print(_time.Minute);
  }

  return 0;
}

uint8_t Page_StbNor::Date(unsigned int color) {
  tft.fillRect(15, 0, 105, 22, ILI9341_BLACK);
  tft.setTextColor(color);
  tft.setFont(&LiberationSans_Regular10pt7b);
  tft.setCursor(15, 15);
  tft.print(day());
  tft.print(" ");
  tft.print(month());
  tft.print(" ");
  tft.print(year());

  return 0;
}

uint8_t Page_StbNor::C0(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[0].x, co[0].y);
  tft.setFont(&LiberationSans_Regular28pt7b);
  tft.print("25.5");
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_StbNor::C1(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[1].x, co[1].y);
  tft.setFont(&LiberationSans_Regular18pt7b);
  tft.print(openweather.GetTemp(), 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_StbNor::C2(unsigned int color) {
  tft.setTextColor(color);

  image_draw->bmpDraw(openweather.getWeatherIcon(), co[2].x, co[2].y);

  return 0;
}

uint8_t Page_StbNor::C3(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[3].x, co[3].y);
  tft.setFont(&LiberationSans_Regular12pt7b);
  tft.print("25.5");
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}
