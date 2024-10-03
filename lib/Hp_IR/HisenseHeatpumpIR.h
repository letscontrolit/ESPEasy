/*
    Hisense AQV12PSBN / AQV09ASA heatpump control (remote control P/N zzz)
*/
#ifndef HisenseHeatpumpIR_h
#define HisenseHeatpumpIR_h

#include <HeatpumpIR.h>


// Hisense timing constants
#define HISENSE_AIRCON1_HDR_MARK   9060
#define HISENSE_AIRCON1_HDR_SPACE  4550
#define HISENSE_AIRCON1_BIT_MARK   520
#define HISENSE_AIRCON1_ONE_SPACE  1700
#define HISENSE_AIRCON1_ZERO_SPACE 630
#define HISENSE_AIRCON1_MSG_SPACE  8140

// Power state
#define HISENSE_AIRCON1_POWER_OFF   0x04	//**
#define HISENSE_AIRCON1_POWER_ON    0x00	//**

// Operating modes
// Hisense codes
#define HISENSE_AIRCON1_MODE_AUTO  0x04 // Not available 0x00
#define HISENSE_AIRCON1_MODE_HEAT  0x00
#define HISENSE_AIRCON1_MODE_COOL  0x02
#define HISENSE_AIRCON1_MODE_DRY   0x03
#define HISENSE_AIRCON1_MODE_FAN   0x04
#define HISENSE_AIRCON1_MODE_MAINT 0x04 // Power OFF

// Fan speeds. Note that some heatpumps have less than 5 fan speeds

#define HISENSE_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define HISENSE_AIRCON1_FAN1       0x03 // * low
#define HISENSE_AIRCON1_FAN2       0x02 // * med
#define HISENSE_AIRCON1_FAN3       0x01 // * high
#define HISENSE_AIRCON1_FAN4       0x01 // * high Not available
#define HISENSE_AIRCON1_FAN5       0x01 // * high Not available

// Not available in this model.
// Vertical air directions. Note that these cannot be set on all heat pumps
#define HISENSE_VDIR_AUTO   0
#define HISENSE_VDIR_MANUAL 0
#define HISENSE_VDIR_SWING  0
#define HISENSE_VDIR_UP     0
#define HISENSE_VDIR_MUP    0
#define HISENSE_VDIR_MIDDLE 0
#define HISENSE_VDIR_MDOWN  0
#define HISENSE_VDIR_DOWN   0

// Not available in this model.
// Horizontal air directions. Note that these cannot be set on all heat pumps
#define HISENSE_HDIR_AUTO   0
#define HISENSE_HDIR_MANUAL 0
#define HISENSE_HDIR_SWING  0
#define HISENSE_HDIR_MIDDLE 0
#define HISENSE_HDIR_LEFT   0
#define HISENSE_HDIR_MLEFT  0
#define HISENSE_HDIR_MRIGHT 0
#define HISENSE_HDIR_RIGHT  0


class HisenseHeatpumpIR : public HeatpumpIR
{
  public:
    HisenseHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendHisense(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif
