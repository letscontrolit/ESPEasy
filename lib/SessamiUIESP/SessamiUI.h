/**************************************************************************************
  Title : Sessami UI parent class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : template parent class for all Sessami UI class
 ***************************************************************************************/

#ifndef SESSAMI_UI_H_
#define SESSAMI_UI_H_

#include "Arduino.h"
#include <ImageDraw.h>
#include "SessamiController.h"
#include <string>

typedef String string;
#include <CAP1114_Button.h>
#include <Fonts.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 16
#define TFT_CS 4

#define SCREENWIDTH   320
#define SCREENHEIGHT  240
struct Coordinates {
  uint16_t x;
  uint16_t y;
};

class SessamiUI {
  protected:
    static bool ui_init;
    static ImageDraw *image_draw; // Old draw library
    static Adafruit_ILI9341 tft;
    uint8_t state;
    static Sessami_Button *button;

  public:
    static void initScr();
    virtual uint8_t UIStateMachine(bool rst) = 0;
    virtual uint8_t EvLoop();
    virtual uint8_t EvSec();
    virtual uint8_t EvMin();
    virtual uint8_t EvHr();

    SessamiUI();
    virtual ~SessamiUI();

  public:
    char* Keyboard();
  private:
    void DrawKeyboard();
};

#endif

