/*
    Marshall / Conqueror heatpump control (remote control P/N R51M/CE)
*/
#ifndef R51MHeatpumpIR_h
#define R51MHeatpumpIR_h

#include <HeatpumpIR.h>

// R51M timing constants
#define R51M_AIRCON1_HDR_MARK   4400
#define R51M_AIRCON1_HDR_SPACE  4150
#define R51M_AIRCON1_BIT_MARK   550
#define R51M_AIRCON1_ONE_SPACE  1520
#define R51M_AIRCON1_ZERO_SPACE 550

#define R51M_AIRCON1_MODE_AUTO  0x08 // Operating mode
#define R51M_AIRCON1_MODE_COOL  0x00
#define R51M_AIRCON1_MODE_FANDRY   0x04
#define R51M_AIRCON1_MODE_OFF   0x7B // Power OFF
#define R51M_AIRCON1_MODE_OFF_TEMP 0xE0
#define R51M_AIRCON1_FAN_AUTO  0xB0 // Fan speed
#define R51M_AIRCON1_FAN_DRY   0x10 // Fan speed
#define R51M_AIRCON1_FAN1       0x90 // * low
#define R51M_AIRCON1_FAN2       0x50 // * med
#define R51M_AIRCON1_FAN3       0x30 // * high

class R51MHeatpumpIR : public HeatpumpIR
{
  public:
    R51MHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif