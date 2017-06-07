/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : Class of Sessami UI page
 ***************************************************************************************/

#include "SessamiUI.h"
#include "Thermostat.h"
#include <OpenWeather_ESP8266.h>

class Page_Normal : private SessamiUI {
  private:
    OpenWeather_ESP8266 openweather;
    Thermostat thermostat;
    Si7020 temp_sensor;

    unsigned long loop_t;
    unsigned long last_t;
    int8_t sub_state;
    bool show_sub;
    Coordinates co[4];

    /* number update */
    float intemp;
    float last_intemp;
    float outtemp;
    float last_outtemp;

    void Main(bool rst);
    void Sub(bool rst);

    uint8_t LoopTime(unsigned int color);
    uint8_t PageTitle(unsigned int color);
    uint8_t Date(unsigned int color);
    uint8_t Time(unsigned int color);

    uint8_t IndoorTemp(unsigned int color);
    uint8_t OutdoorTemp(unsigned int color);
    uint8_t ThermostatSetPt(unsigned int color);

    uint8_t C0(unsigned int color);
    uint8_t C1(unsigned int color);
    uint8_t C2(unsigned int color);

  public:
    virtual uint8_t EvMin();
    virtual uint8_t EvHr();
    virtual uint8_t UIStateMachine(bool rst);

    Page_Normal();
    virtual ~Page_Normal();
};

Page_Normal::Page_Normal() :  sub_state(0), show_sub(true), loop_t(0), last_t(0) {
  co[0].x = 247; co[0].y = 70;
  co[1].x = 25; co[1].y = 100;
  co[2].x = 50; co[2].y = 110;
  co[3].x = 140; co[3].y = 135;
}

Page_Normal::~Page_Normal() {

}

uint8_t Page_Normal::EvMin() {
  Time(ILI9341_WHITE);
}

uint8_t Page_Normal::EvHr() {
  Date(ILI9341_WHITE);
}

uint8_t Page_Normal::UIStateMachine(bool rst) {
  if (rst) {
    last_intemp = intemp = temp_sensor.GetTp();
    last_outtemp = outtemp = openweather.GetTemp();
    
    //image_draw->ClearLCD();
    tft.fillRect(0, 15, SCREENWIDTH, SCREENHEIGHT - 15, ILI9341_BLACK);
    PageTitle(ILI9341_WHITE);
    Date(ILI9341_WHITE);
    Time(ILI9341_WHITE);
    //state = 0;
  }
  LoopTime(ILI9341_WHITE);

  Main(rst);
  if (show_sub)
    Sub(rst);

  return 0;
}

void Page_Normal::Main(bool rst) {
  switch (state) {
    case 0 :
      //Use Standby
      if (rst) {
        IndoorTemp(ILI9341_WHITE);
        OutdoorTemp(ILI9341_WHITE);
        ThermostatSetPt(ILI9341_WHITE);
      }
      
      intemp = temp_sensor.GetTp();
      outtemp = openweather.GetTemp();
      if (intemp != last_intemp) {
        IndoorTemp(ILI9341_BLACK);
        last_intemp = intemp;
        IndoorTemp(ILI9341_WHITE);
      }
      if (outtemp != last_outtemp) {
        OutdoorTemp(ILI9341_BLACK);
        last_outtemp = outtemp;
        OutdoorTemp(ILI9341_WHITE);
      }
      if (*button == B_UP) {
        ThermostatSetPt(ILI9341_BLACK);
        thermostat++;
        ThermostatSetPt(ILI9341_WHITE);
      } else if (*button == B_DOWN) {
        ThermostatSetPt(ILI9341_BLACK);
        thermostat--;
        ThermostatSetPt(ILI9341_WHITE);
      }
      break;
    case 1 :
      break;
    case 2 :
      break;
  }
}

void Page_Normal::Sub(bool rst) {
  if (*button == S_RIGHT) {
    sub_state++;
    if (sub_state > 2)
      sub_state = 0;
    rst = true;
  } else if (*button == S_LEFT) {
    sub_state--;
    if (sub_state < 0)
      sub_state = 2;
    rst = true;
  }
  switch (sub_state) {
    case 0 :
      //print icon
      if (rst) {
        C0(ILI9341_WHITE);
        C1(ILI9341_BLACK);
        C2(ILI9341_BLACK);
      }
      break;
    case 1 :
      if (rst) {
        C0(ILI9341_WHITE);
        C1(ILI9341_WHITE);
        C2(ILI9341_BLACK);
      }
      break;
    case 2 :
      if (rst) {
        C0(ILI9341_WHITE);
        C1(ILI9341_WHITE);
        C2(ILI9341_WHITE);
      }
      break;
  }

  /*if (*button == S_TAP) {
    state = sub_state;
  }*/
}

uint8_t Page_Normal::PageTitle(unsigned int color) {
  tft.fillRect(235, 0, 100, 13, ILI9341_BLACK);
  tft.setTextColor(color);

  tft.setFont(&LiberationSans_Regular6pt7b);
  tft.setCursor(250, 10);
  tft.print("Main");

  return 0;
}

uint8_t Page_Normal::LoopTime(unsigned int color) {
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

uint8_t Page_Normal::Time(unsigned int color) {
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

uint8_t Page_Normal::Date(unsigned int color) {
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

uint8_t Page_Normal::C0(unsigned int color) {
  tft.drawCircle(50, 211, 25, color);
  tft.drawCircle(160, 211, 25, color);
  tft.drawCircle(270, 211, 25, color);
}

uint8_t Page_Normal::C1(unsigned int color) {
  tft.drawCircle(50, 211, 17, color);
  tft.drawCircle(160, 211, 17, color);
  tft.drawCircle(270, 211, 17, color);
}

uint8_t Page_Normal::C2(unsigned int color) {
  tft.drawCircle(50, 211, 9, color);
  tft.drawCircle(160, 211, 9, color);
  tft.drawCircle(270, 211, 9, color);
}

/*******************************************************************
                            Standby Screen
*/

uint8_t Page_Normal::IndoorTemp(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[0].x, co[0].y);
  tft.setFont(&LiberationSans_Regular12pt7b);
  tft.print(last_intemp, 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_Normal::OutdoorTemp(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[1].x, co[1].y);
  tft.setFont(&LiberationSans_Regular24pt7b);
  tft.print(last_outtemp, 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_Normal::ThermostatSetPt(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[3].x, co[3].y);
  tft.setFont(&LiberationSans_Regular40pt7b);
  tft.print(thermostat.GetTempSetPt(), 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}
