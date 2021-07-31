/*
    Daikin heatpump control (Daikin remote control P/N ARC417A3)
*/
#ifndef DaikinHeatpumpARC417IR_h
#define DaikinHeatpumpARC417IR_h

#include <HeatpumpIR.h>


// Daikin timing constants
#define DAIKIN_AIRCON2_HDR_MARK   5050
#define DAIKIN_AIRCON2_HDR_SPACE  2100
#define DAIKIN_AIRCON2_BIT_MARK   391
#define DAIKIN_AIRCON2_ONE_SPACE  1725
#define DAIKIN_AIRCON2_ZERO_SPACE 667
#define DAIKIN_AIRCON2_MSG_SPACE  30000

// Daikin codes
#define DAIKIN_AIRCON2_MODE_AUTO  0x10 // Operating mode
#define DAIKIN_AIRCON2_MODE_HEAT  0x40
#define DAIKIN_AIRCON2_MODE_COOL  0x30
#define DAIKIN_AIRCON2_MODE_DRY   0x20
#define DAIKIN_AIRCON2_MODE_FAN   0x60
#define DAIKIN_AIRCON2_MODE_OFF   0x00 // Power OFF
#define DAIKIN_AIRCON2_MODE_ON    0x01
#define DAIKIN_AIRCON2_FAN_AUTO   0x0A // Fan speed
#define DAIKIN_AIRCON2_FAN1       0x03
#define DAIKIN_AIRCON2_FAN2       0x04
#define DAIKIN_AIRCON2_FAN3       0x05
#define DAIKIN_AIRCON2_FAN4       0x06
#define DAIKIN_AIRCON2_FAN5       0x07


class DaikinHeatpumpARC417IR : public HeatpumpIR
{
  public:
    DaikinHeatpumpARC417IR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif