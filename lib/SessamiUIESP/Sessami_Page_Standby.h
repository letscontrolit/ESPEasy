/**************************************************************************************
  Title : Sessami UI Page class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : Class of Sessami UI page
 ***************************************************************************************/

#include "SessamiUI.h"
#include <OpenWeather_ESP8266.h>
#include "Thermostat.h"
#include <Si7020.h>

extern tmElements_t _time;

class Page_Standby : private SessamiUI {
  private:
    unsigned long loop_t;
    unsigned long last_t;
    OpenWeather_ESP8266 openweather;
    Si7020 temp_sensor;
    Thermostat thermostat;
    Coordinates co[4];

    /* number update */
    float intemp;
    float last_intemp;
    float outtemp;
    float last_outtemp;

    uint8_t PageTitle(unsigned int color);
    uint8_t LoopTime(unsigned int color);
    uint8_t Time(unsigned int color);
    uint8_t Date(unsigned int color);
    uint8_t IndoorTemp(unsigned int color);
    uint8_t OutdoorTemp(unsigned int color);
    uint8_t C2(unsigned int color);
    uint8_t ThermostatSetPt(unsigned int color);
    
    
  public:
    virtual uint8_t EvMin();
    virtual uint8_t EvHr();
    virtual uint8_t UIStateMachine(bool rst);

    Page_Standby();
    virtual ~Page_Standby();
};

Page_Standby::Page_Standby() {
  co[0].x = 140; co[0].y = 100;
  co[1].x = 25; co[1].y = 100;
  co[2].x = 50; co[2].y = 110;
  co[3].x = 237; co[3].y = 125;
}

Page_Standby::~Page_Standby() {

}

uint8_t Page_Standby::EvMin() {
  Time(ILI9341_WHITE);
}

uint8_t Page_Standby::EvHr() {
  Date(ILI9341_WHITE);
}

uint8_t Page_Standby::UIStateMachine(bool rst) {
  if (rst) {
    last_intemp = intemp = temp_sensor.GetTp();
    last_outtemp = outtemp = openweather.GetTemp();
    
    //image_draw->ClearLCD();
    tft.fillRect(0, 15, SCREENWIDTH, SCREENHEIGHT - 15, ILI9341_BLACK);
    PageTitle(ILI9341_WHITE);
    Date(ILI9341_WHITE);
    Time(ILI9341_WHITE);
    IndoorTemp(ILI9341_WHITE);
    OutdoorTemp(ILI9341_WHITE);
    C2(ILI9341_WHITE);
    ThermostatSetPt(ILI9341_WHITE);
    
    state = 0;
  }
  LoopTime(ILI9341_WHITE);

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
  return 0;
}

uint8_t Page_Standby::PageTitle(unsigned int color) {
  tft.fillRect(235, 0, 100, 13, ILI9341_BLACK);
  tft.setTextColor(color);

  tft.setFont(&LiberationSans_Regular6pt7b);
  tft.setCursor(235, 10);
  tft.print("Standby");

  return 0;
}

uint8_t Page_Standby::LoopTime(unsigned int color) {
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

uint8_t Page_Standby::Time(unsigned int color) {
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

uint8_t Page_Standby::Date(unsigned int color) {
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

uint8_t Page_Standby::IndoorTemp(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[0].x, co[0].y);
  tft.setFont(&LiberationSans_Regular40pt7b);
  tft.print(last_intemp, 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_Standby::OutdoorTemp(unsigned int color) {
  tft.setTextColor(color);
  
  tft.setCursor(co[1].x, co[1].y);
  tft.setFont(&LiberationSans_Regular24pt7b);
  tft.print(last_outtemp, 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}

uint8_t Page_Standby::C2(unsigned int color) {
  tft.setTextColor(color);

  image_draw->bmpDraw(openweather.getWeatherIcon(), co[2].x, co[2].y);
  
  return 0;
}

uint8_t Page_Standby::ThermostatSetPt(unsigned int color) {
  tft.setTextColor(color);

  tft.setCursor(co[3].x, co[3].y);
  tft.setFont(&LiberationSans_Regular12pt7b);
  tft.print(thermostat.GetTempSetPt(), 1);
  tft.setFont(&LiberationSans_Regular4pt7b);
  tft.print(" o");

  return 0;
}
