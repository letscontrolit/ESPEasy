/*
    Panasonic DKE/JKE/NKE heatpump control (DKE remote control P/N A75C2616 etc)
*/
#ifndef PanasonicHeatpumpIR_h
#define PanasonicHeatpumpIR_h

#include <HeatpumpIR.h>


// Panasonic DKE, JKE & NKE timing constants (DKE remote control P/N A75C2616)
#define PANASONIC_AIRCON2_HDR_MARK   3500
#define PANASONIC_AIRCON2_HDR_SPACE  1800
#define PANASONIC_AIRCON2_BIT_MARK   420
#define PANASONIC_AIRCON2_ONE_SPACE  1350
#define PANASONIC_AIRCON2_ZERO_SPACE 470
#define PANASONIC_AIRCON2_MSG_SPACE  10000

// Panasonic DKE, JNE & NKE codes
#define PANASONIC_AIRCON2_MODE_AUTO  0x00 // Operating mode
#define PANASONIC_AIRCON2_MODE_HEAT  0x40
#define PANASONIC_AIRCON2_MODE_COOL  0x30
#define PANASONIC_AIRCON2_MODE_DRY   0x20
#define PANASONIC_AIRCON2_MODE_FAN   0x60
#define PANASONIC_AIRCON2_MODE_OFF   0x00 // Power OFF
#define PANASONIC_AIRCON2_MODE_ON    0x01
#define PANASONIC_AIRCON2_TIMER_CNL  0x08
#define PANASONIC_AIRCON2_FAN_AUTO   0xA0 // Fan speed
#define PANASONIC_AIRCON2_FAN1       0x30
#define PANASONIC_AIRCON2_FAN2       0x40
#define PANASONIC_AIRCON2_FAN3       0x50
#define PANASONIC_AIRCON2_FAN4       0x60
#define PANASONIC_AIRCON2_FAN5       0x70
#define PANASONIC_AIRCON2_VS_AUTO    0x0F // Vertical swing
#define PANASONIC_AIRCON2_VS_UP      0x01
#define PANASONIC_AIRCON2_VS_MUP     0x02
#define PANASONIC_AIRCON2_VS_MIDDLE  0x03
#define PANASONIC_AIRCON2_VS_MDOWN   0x04
#define PANASONIC_AIRCON2_VS_DOWN    0x05
#define PANASONIC_AIRCON2_HS_AUTO    0x0D // Horizontal swing
#define PANASONIC_AIRCON2_HS_MIDDLE  0x06
#define PANASONIC_AIRCON2_HS_LEFT    0x09
#define PANASONIC_AIRCON2_HS_MLEFT   0x0A
#define PANASONIC_AIRCON2_HS_MRIGHT  0x0B
#define PANASONIC_AIRCON2_HS_RIGHT   0x0C
#define PANASONIC_AIRCON2_QUIET      0x01 // Quiet setting
#define PANASONIC_AIRCON2_POWERFUL   0x20 // Powerful setting

// Panasonic model codes
#define PANASONIC_DKE 0
#define PANASONIC_JKE 1
#define PANASONIC_NKE 2
#define PANASONIC_LKE 3


class PanasonicHeatpumpIR : public HeatpumpIR
{
  protected: // Cannot create generic Panasonic heatpump instances
    PanasonicHeatpumpIR();
    uint8_t _panasonicModel;  // Tells whether this is DKE, NKE or JKE (or other supported model...)

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool powerfulCmd, bool quietCmd);

  private:
    void sendPanasonic(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t profile);
};

class PanasonicDKEHeatpumpIR : public PanasonicHeatpumpIR
{
  public:
    PanasonicDKEHeatpumpIR();
};

class PanasonicJKEHeatpumpIR : public PanasonicHeatpumpIR
{
  public:
    PanasonicJKEHeatpumpIR();
};

class PanasonicNKEHeatpumpIR : public PanasonicHeatpumpIR
{
  public:
    PanasonicNKEHeatpumpIR();
};

class PanasonicLKEHeatpumpIR : public PanasonicHeatpumpIR
{
  public:
    PanasonicLKEHeatpumpIR();
};

#endif