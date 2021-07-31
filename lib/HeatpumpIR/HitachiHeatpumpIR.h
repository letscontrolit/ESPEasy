/*
		Hitachi RAR-5E1 remote
*/
#ifndef HitachiHeatpumpIR_h
#define HitachiHeatpumpIR_h

#include <HeatpumpIR.h>

// Hitachi remote timing constants
#define HITACHI_AIRCON1_HDR_MARK   3436
#define HITACHI_AIRCON1_HDR_SPACE  1640
#define HITACHI_AIRCON1_BIT_MARK   420
#define HITACHI_AIRCON1_ONE_SPACE  1250
#define HITACHI_AIRCON1_ZERO_SPACE 500

// Hitachi codes
#define HITACHI_AIRCON1_MODE_AUTO  0x02 // Operating mode
#define HITACHI_AIRCON1_MODE_HEAT  0x03
#define HITACHI_AIRCON1_MODE_COOL  0x04
#define HITACHI_AIRCON1_MODE_DRY   0x05
#define HITACHI_AIRCON1_MODE_FAN   0x0C

#define HITACHI_AIRCON1_POWER_OFF  0x00 // Power OFF
#define HITACHI_AIRCON1_POWER_ON   0x80

#define HITACHI_AIRCON1_FAN_AUTO   0x01 // Fan speed
#define HITACHI_AIRCON1_FAN1       0x02
#define HITACHI_AIRCON1_FAN2       0x03
#define HITACHI_AIRCON1_FAN3       0x04
#define HITACHI_AIRCON1_FAN4       0x05

#define HITACHI_AIRCON1_VDIR_AUTO  0x00
#define HITACHI_AIRCON1_VDIR_SWING 0x01

#define HITACHI_AIRCON1_HDIR_AUTO  0x00
#define HITACHI_AIRCON1_HDIR_SWING 0x01

class HitachiHeatpumpIR : public HeatpumpIR
{
  public:
    HitachiHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVcmd, uint8_t swingHcmd);

  private:
    void sendHitachi(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif
