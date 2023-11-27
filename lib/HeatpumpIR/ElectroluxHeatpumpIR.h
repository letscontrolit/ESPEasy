/*
    Electrolux heatpump control (remote control YAL1F) Based on Gree YAC
*/
#ifndef ElectroluxHeatpumpIR_h
#define ElectroluxHeatpumpIR_h

#include <HeatpumpIR.h>

// Electrolux timing constants
#define ELECTROLUX_AIRCON1_HDR_MARK   9000
#define ELECTROLUX_AIRCON1_HDR_SPACE  4000
#define ELECTROLUX_AIRCON1_BIT_MARK   620
#define ELECTROLUX_AIRCON1_ONE_SPACE  1600
#define ELECTROLUX_AIRCON1_ZERO_SPACE 540
#define ELECTROLUX_AIRCON1_MSG_SPACE  19000


// Power state
#define ELECTROLUX_AIRCON1_POWER_OFF  0x00
#define ELECTROLUX_AIRCON1_POWER_ON   0x08

// Operating modes
// Electrolux codes
#define ELECTROLUX_AIRCON1_MODE_AUTO  0x00
#define ELECTROLUX_AIRCON1_MODE_COOL  0x01
#define ELECTROLUX_AIRCON1_MODE_DRY   0x02
#define ELECTROLUX_AIRCON1_MODE_FAN   0x03
#define ELECTROLUX_AIRCON1_MODE_HEAT  0x04

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define ELECTROLUX_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define ELECTROLUX_AIRCON1_FAN1       0x10 // * 1
#define ELECTROLUX_AIRCON1_FAN2       0x20 // * 2
#define ELECTROLUX_AIRCON1_FAN3       0x30 // * 3

// Vertical air directions. Note that these cannot be set on all heat pumps
#define ELECTROLUX_VDIR_AUTO   0x00
#define ELECTROLUX_VDIR_SWING  0x01

// Horizontal air directions. Note that these cannot be set on all heat pumps
#define ELECTROLUX_HDIR_AUTO   0x00
#define ELECTROLUX_HDIR_SWING  0x01

#define ELECTROLUX_YAL     0


class ElectroluxHeatpumpIR : public HeatpumpIR
{
  protected:
    ElectroluxHeatpumpIR();
    uint8_t electroluxModel;

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendElectrolux(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};


class ElectroluxYALHeatpumpIR : public ElectroluxHeatpumpIR
{
  public:
    ElectroluxYALHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
    {
      ElectroluxHeatpumpIR::send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd);
    }
};

#endif
