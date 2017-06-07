/**************************************************************************************
  Title : Sessami CAP1114 Button class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : class for handle button state
 ***************************************************************************************/

#ifndef SESSAMI_BUTTON_H_
#define SESSAMI_BUTTON_H_

#include "CAP1114_Driver.h"

using namespace CAP1114;

#define B_PROX 1 //Sensor 1
#define B_MID 32 //Sensor 6
#define B_LEFT 64 //Sensor 7
#define B_UP 2 //Sensor 2
#define B_POWER 8 //Sensor 4
#define B_DOWN 4 //Sensor 3
#define B_RIGHT 16 //Sensor 5

//#define S_PH 257 //256 + 1  change to bool
//#define S_TAP 258 //256 + 2  change to bool
#define S_RIGHT 257 //256 +1
#define S_LEFT 258 //256 + 2
//#define S_RESET 288 //256+32
//#define S_MULT 320 //256+64

class Sessami_Button: private CAP1114_Driver {
  private:
    static uint8_t button_state;
    static uint8_t slide_state;
    static unsigned long held_t;
    static unsigned long button_hold_t;
    static unsigned int button_tap;
    static bool slide_tap;
    static bool slide_ph;
    static uint8_t delta_sen;
    static uint8_t threshold[8];
    static int8_t delta_count[8];
  public:
    Sessami_Button();
    ~Sessami_Button();

    bool operator==(const unsigned int key) const;

    void UpdateBut();
    uint8_t GetBut();
    uint8_t GetSli();
    unsigned long GetHeldT();
    void HeldCount();
    void HoldCount();
    bool BuTap();

    //42h Prox Sensitivity
    void SetPROXSen(uint8_t value);
    uint8_t GetPROXSen();
    bool GetPROXEn();
    //1Fh Data Sensitivity
    void SetButSen(uint8_t value);
    uint8_t GetButSen();
    //10h-1Dh Delta Count
    int8_t GetDeltaCount(uint8_t key);
    //30h-37h Sensor Threshold
    uint8_t GetTh(uint8_t key);
    uint8_t SetTh(uint8_t key, uint8_t value);
    uint8_t Getthreshold(uint8_t key); //tmp
};

#endif
