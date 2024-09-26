/*
    Philco PHS32 heatpump control
*/
#ifndef PhilcoPHS32HeatpumpIR_h
#define PhilcoPHS32HeatpumpIR_h

#include <HeatpumpIR.h>

// Timing constants
#define PHILCO_AIRCON1_HDR_MARK   9000
#define PHILCO_AIRCON1_HDR_SPACE  4500
#define PHILCO_AIRCON1_MSG_SPACE  8000
#define PHILCO_AIRCON1_BIT_MARK   562
#define PHILCO_AIRCON1_ONE_SPACE  1687
#define PHILCO_AIRCON1_ZERO_SPACE 562

// Power states
#define PHILCO_AIRCON1_POWER_ON   0x30
#define PHILCO_AIRCON1_POWER_OFF  0x20

// Mode temperatures
#define PHILCO_AIRCON1_OFF_TEMP   0x12
#define PHILCO_AIRCON1_DRY_TEMP   0x19
#define PHILCO_AIRCON1_FAN_TEMP   0x19

// Operating modes
#define PHILCO_AIRCON1_MODE_HEAT  0x00
#define PHILCO_AIRCON1_MODE_COOL  0x02
#define PHILCO_AIRCON1_MODE_DRY   0x03
#define PHILCO_AIRCON1_MODE_FAN   0x04

// Fan speeds
#define PHILCO_AIRCON1_FAN_AUTO   0x00 // * auto
#define PHILCO_AIRCON1_FAN_LOW    0x03 // * low
#define PHILCO_AIRCON1_FAN_MED    0x02 // * med
#define PHILCO_AIRCON1_FAN_HIGH   0x01 // * high

class PhilcoPHS32HeatpumpIR : public HeatpumpIR
{
  public:
    PhilcoPHS32HeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendPhilco(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif
