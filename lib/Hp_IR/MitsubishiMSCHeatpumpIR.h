/*
   Mitsubishi Electric MSC-GA20VB, MSC-GA25VB, MSC-GA35VB (remote control P/N KP1A)
*/

#ifndef MitsubishiMSCHeatpumpIR_h
#define MitsubishiMSCHeatpumpIR_h

#include <HeatpumpIR.h>

// Mitsubishi MSC timing constants
#define MITSUBISHIMSC_AIRCON1_HDR_MARK   3060
#define MITSUBISHIMSC_AIRCON1_HDR_SPACE  1580
#define MITSUBISHIMSC_AIRCON1_BIT_MARK   350
#define MITSUBISHIMSC_AIRCON1_ONE_SPACE  1150
#define MITSUBISHIMSC_AIRCON1_ZERO_SPACE 390
#define MITSUBISHIMSC_AIRCON1_MSG_SPACE  0

// Mitsubishi MSC codes
#define MITSUBISHIMSC_AIRCON1_MODE_AUTO      0x08 // Operating mode
#define MITSUBISHIMSC_AIRCON1_MODE_HEAT      0x01
#define MITSUBISHIMSC_AIRCON1_MODE_COOL      0x03
#define MITSUBISHIMSC_AIRCON1_MODE_DRY       0x02
#define MITSUBISHIMSC_AIRCON1_MODE_FAN       0x07
#define MITSUBISHIMSC_AIRCON1_MODE_OFF       0x20 // Power OFF
#define MITSUBISHIMSC_AIRCON1_MODE_ON        0x24 // Power ON
#define MITSUBISHIMSC_AIRCON1_FAN_AUTO       0x00 // Fan speeds
#define MITSUBISHIMSC_AIRCON1_FAN1           0x02
#define MITSUBISHIMSC_AIRCON1_FAN2           0x03
#define MITSUBISHIMSC_AIRCON1_FAN3           0x05
#define MITSUBISHIMSC_AIRCON1_VS_SWING       0x38 // Vertical swing
#define MITSUBISHIMSC_AIRCON1_VS_AUTO        0x00
#define MITSUBISHIMSC_AIRCON1_VS_UP          0x08
#define MITSUBISHIMSC_AIRCON1_VS_MUP         0x10
#define MITSUBISHIMSC_AIRCON1_VS_MIDDLE      0x18
#define MITSUBISHIMSC_AIRCON1_VS_MDOWN       0x20
#define MITSUBISHIMSC_AIRCON1_VS_DOWN        0x28


class MitsubishiMSCHeatpumpIR : public HeatpumpIR
{
  public:
    MitsubishiMSCHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendMitsubishiMSC(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
