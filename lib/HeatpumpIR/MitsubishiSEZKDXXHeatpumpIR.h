/*
   Mitsubishi SEZ series SEZ_KD_VAL23/35 and so on remote W001CP
*/

#ifndef MitsubishiSEZKDXXHeatpumpIR_h
#define MitsubishiSEZKDXXHeatpumpIR_h

#include <HeatpumpIR.h>

// Mitsubishi SEZ timing constants
#define MITSUBISHISEZ_AIRCON1_HDR_MARK   3060
#define MITSUBISHISEZ_AIRCON1_HDR_SPACE  1580
#define MITSUBISHISEZ_AIRCON1_BIT_MARK   350
#define MITSUBISHISEZ_AIRCON1_ONE_SPACE  1150
#define MITSUBISHISEZ_AIRCON1_ZERO_SPACE 390
#define MITSUBISHISEZ_AIRCON1_MSG_SPACE  0


// Mitsubishi SEZ codes
#define MITSUBISHISEZ_AIRCON1_MODE_AUTO      0x03 // Operating mode
#define MITSUBISHISEZ_AIRCON1_MODE_HEAT      0x02
#define MITSUBISHISEZ_AIRCON1_MODE_COOL      0x01
#define MITSUBISHISEZ_AIRCON1_MODE_DRY       0x05
#define MITSUBISHISEZ_AIRCON1_MODE_FAN       0x00
#define MITSUBISHISEZ_AIRCON1_MODE_OFF       0x00 // Power OFF - not real codes, but we need something...
#define MITSUBISHISEZ_AIRCON1_MODE_ON        0x40 // Power ON
#define MITSUBISHISEZ_AIRCON1_FAN1           0x32
#define MITSUBISHISEZ_AIRCON1_FAN2           0x34
#define MITSUBISHISEZ_AIRCON1_FAN3           0x36


class MitsubishiSEZKDXXHeatpumpIR : public HeatpumpIR
{
  public:
    MitsubishiSEZKDXXHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendMitsubishiSEZKDXX(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
