/*
    Gree heatpump control (remote control P/N zzz)
*/
#ifndef GreeHeatpumpIR_h
#define GreeHeatpumpIR_h

#include <HeatpumpIR.h>

// Gree timing constants
#define GREE_AIRCON1_HDR_MARK   9000
#define GREE_AIRCON1_HDR_SPACE  4000
#define GREE_AIRCON1_BIT_MARK   620
#define GREE_AIRCON1_ONE_SPACE  1600
#define GREE_AIRCON1_ZERO_SPACE 540
#define GREE_AIRCON1_MSG_SPACE  19000

// Power state
#define GREE_AIRCON1_POWER_OFF  0x00
#define GREE_AIRCON1_POWER_ON   0x08

// Operating modes
// Gree codes
#define GREE_AIRCON1_MODE_AUTO  0x00
#define GREE_AIRCON1_MODE_COOL  0x01
#define GREE_AIRCON1_MODE_DRY   0x02
#define GREE_AIRCON1_MODE_FAN   0x03
#define GREE_AIRCON1_MODE_HEAT  0x04

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define GREE_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define GREE_AIRCON1_FAN1       0x10 // * low
#define GREE_AIRCON1_FAN2       0x20 // * med
#define GREE_AIRCON1_FAN3       0x30 // * high
#define GREE_AIRCON1_TURBO      0x80 // * turbo mode on YAN

// Only available on YAN
// Vertical air directions. Note that these cannot be set on all heat pumps
#define GREE_VDIR_AUTO   0x00
#define GREE_VDIR_MANUAL 0x00
#define GREE_VDIR_SWING  0x01
#define GREE_VDIR_UP     0x02
#define GREE_VDIR_MUP    0x03
#define GREE_VDIR_MIDDLE 0x04
#define GREE_VDIR_MDOWN  0x05
#define GREE_VDIR_DOWN   0x06

// Not available in this model.
// Horizontal air directions. Note that these cannot be set on all heat pumps
#define GREE_HDIR_AUTO   0
#define GREE_HDIR_MANUAL 0
#define GREE_HDIR_SWING  0
#define GREE_HDIR_MIDDLE 0
#define GREE_HDIR_LEFT   0
#define GREE_HDIR_MLEFT  0
#define GREE_HDIR_MRIGHT 0
#define GREE_HDIR_RIGHT  0

// Gree model codes
#define GREE_GENERIC 0
#define GREE_YAN     1
#define GREE_YAA     2


class GreeHeatpumpIR : public HeatpumpIR
{
  protected:
    GreeHeatpumpIR();
    uint8_t greeModel;

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd , uint8_t fanSpeedCmd , uint8_t temperatureCmd , uint8_t swingVCmd , uint8_t swingHCmd, bool turboMode);

  private:
    void sendGree(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, bool turboMode);
};

class GreeGenericHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeGenericHeatpumpIR();
};

class GreeYANHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeYANHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode);
};

class GreeYAAHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeYAAHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode);
};

#endif
