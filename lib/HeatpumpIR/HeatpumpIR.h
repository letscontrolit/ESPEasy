/*
    The generic heatpump interface
    * Constants for modes
    * The constructor and the 'send' command interface
*/

#ifndef HeatpumpIR_h
#define HeatpumpIR_h

#include <Arduino.h>
#include <IRSender.h>


// Power state
#define POWER_OFF   0
#define POWER_ON    1

// Operating modes
#define MODE_AUTO   1
#define MODE_HEAT   2
#define MODE_COOL   3
#define MODE_DRY    4
#define MODE_FAN    5
#define MODE_MAINT  6

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define FAN_AUTO    0
#define FAN_1       1
#define FAN_2       2
#define FAN_3       3
#define FAN_4       4
#define FAN_5       5

// Vertical air directions. Note that these cannot be set on all heat pumps
#define VDIR_AUTO   0
#define VDIR_MANUAL 0
#define VDIR_SWING  1
#define VDIR_UP     2
#define VDIR_MUP    3
#define VDIR_MIDDLE 4
#define VDIR_MDOWN  5
#define VDIR_DOWN   6

// Horizontal air directions. Note that these cannot be set on all heat pumps
#define HDIR_AUTO   0
#define HDIR_MANUAL 0
#define HDIR_SWING  1
#define HDIR_MIDDLE 2
#define HDIR_LEFT   3
#define HDIR_MLEFT  4
#define HDIR_MRIGHT 5
#define HDIR_RIGHT  6


class HeatpumpIR
{
  protected:
    HeatpumpIR(); // Cannot create generic heatpump instances
    const char * _model;
    const char * _info;

  public:
    virtual void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    const char PROGMEM* model();
    const char PROGMEM* info();
};

#endif
