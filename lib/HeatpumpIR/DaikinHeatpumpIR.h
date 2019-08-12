/*
    Daikin heatpump control (Daikin remote control P/N ARC433B50)
*/
#ifndef DaikinHeatpumpIR_h
#define DaikinHeatpumpIR_h

#include <HeatpumpIR.h>


// Daikin timing constants
#define DAIKIN_AIRCON_HDR_MARK   3360  // 3300
#define DAIKIN_AIRCON_HDR_SPACE  1760  // 1600
#define DAIKIN_AIRCON_BIT_MARK   360   // 310
#define DAIKIN_AIRCON_ONE_SPACE  1370  // 1220
#define DAIKIN_AIRCON_ZERO_SPACE 520   // 400
#define DAIKIN_AIRCON_MSG_SPACE  32300 // 30800

// Daikin codes
#define DAIKIN_AIRCON_MODE_AUTO  0x00 // Operating mode
#define DAIKIN_AIRCON_MODE_HEAT  0x40
#define DAIKIN_AIRCON_MODE_COOL  0x30
#define DAIKIN_AIRCON_MODE_DRY   0x20
#define DAIKIN_AIRCON_MODE_FAN   0x60
#define DAIKIN_AIRCON_MODE_OFF   0x00 // Power OFF
#define DAIKIN_AIRCON_MODE_ON    0x01
#define DAIKIN_AIRCON_FAN_AUTO   0xA0 // Fan speed
#define DAIKIN_AIRCON_FAN1       0x30
#define DAIKIN_AIRCON_FAN2       0x40
#define DAIKIN_AIRCON_FAN3       0x50
#define DAIKIN_AIRCON_FAN4       0x60
#define DAIKIN_AIRCON_FAN5       0x70


class DaikinHeatpumpIR : public HeatpumpIR
{
  public:
    DaikinHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif