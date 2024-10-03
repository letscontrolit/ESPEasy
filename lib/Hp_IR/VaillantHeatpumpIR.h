/*
    Vaillant VAI8 heatpump control (remote control P/N YAN1F1)
*/
#ifndef VaillantHeatpumpIR_h
#define VaillantHeatpumpIR_h

#include <HeatpumpIR.h>

// Vaillant timing constants
#define VAILLANT_AIRCON1_HDR_MARK   9000
#define VAILLANT_AIRCON1_HDR_SPACE  4000
#define VAILLANT_AIRCON1_BIT_MARK   620
#define VAILLANT_AIRCON1_ONE_SPACE  1600
#define VAILLANT_AIRCON1_ZERO_SPACE 540
#define VAILLANT_AIRCON1_MSG_SPACE  19000

// Power state
#define VAILLANT_AIRCON1_POWER_OFF  0x00
#define VAILLANT_AIRCON1_POWER_ON   0x08

// Operating modes
#define VAILLANT_AIRCON1_MODE_AUTO  0x00
#define VAILLANT_AIRCON1_MODE_COOL  0x01
#define VAILLANT_AIRCON1_MODE_DRY   0x02
#define VAILLANT_AIRCON1_MODE_FAN   0x03
#define VAILLANT_AIRCON1_MODE_HEAT  0x04

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define VAILLANT_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define VAILLANT_AIRCON1_FAN1       0x10 // * low
#define VAILLANT_AIRCON1_FAN2       0x20 // * med
#define VAILLANT_AIRCON1_FAN3       0x30 // * high

// Vertical air directions. Note that these cannot be set on all heat pumps
#define VAILLANT_VDIR_AUTO   0x00
#define VAILLANT_VDIR_MANUAL 0x00
#define VAILLANT_VDIR_SWING  0x01
#define VAILLANT_VDIR_UP     0x02
#define VAILLANT_VDIR_MUP    0x03
#define VAILLANT_VDIR_MIDDLE 0x04
#define VAILLANT_VDIR_MDOWN  0x05
#define VAILLANT_VDIR_DOWN   0x06


class VaillantHeatpumpIR : public HeatpumpIR
{
  public:
    VaillantHeatpumpIR();
    using HeatpumpIR::send;
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, bool turboModeCmd, bool lightCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd) override;
    
  private:
    void sendVaillant(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, bool turboMode, bool light);
};

#endif
