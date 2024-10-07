/*
    AIRWAY etc. (Vivax, Classe, NEO, Galanz, Simbio, Beko, Sintech) heatpump control (remote control P/N GZ-1002B-E3)
    
*/
#ifndef AIRWAYHeatpumpIR_h
#define AIRWAYHeatpumpIR_h

#include <HeatpumpIR.h>


//AIRWAY timing constants
#define AIRWAY_AIRCON1_HDR_MARK   3080
#define AIRWAY_AIRCON1_HDR_SPACE  1700
#define AIRWAY_AIRCON1_BIT_MARK   400
#define AIRWAY_AIRCON1_ONE_SPACE  1060
#define AIRWAY_AIRCON1_ZERO_SPACE 320

// AIRWAY codes
#define AIRWAY_AIRCON1_MODE_AUTO  0x08 // Operating mode
#define AIRWAY_AIRCON1_MODE_HEAT  0x01
#define AIRWAY_AIRCON1_MODE_COOL  0x03
#define AIRWAY_AIRCON1_MODE_DRY   0x02
#define AIRWAY_AIRCON1_MODE_FAN   0x07
#define AIRWAY_AIRCON1_MODE_ON    0x04 // Power ON
#define AIRWAY_AIRCON1_MODE_OFF   0x00
#define AIRWAY_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define AIRWAY_AIRCON1_FAN1       0x02
#define AIRWAY_AIRCON1_FAN2       0x03
#define AIRWAY_AIRCON1_FAN3       0x05

#define AIRWAY_AIRCON1_VS_AUTO    0x00 // Vertical swing
#define AIRWAY_AIRCON1_VS_UP      0x08
#define AIRWAY_AIRCON1_VS_MUP     0x10
#define AIRWAY_AIRCON1_VS_MIDDLE  0x18
#define AIRWAY_AIRCON1_VS_MDOWN   0x20
#define AIRWAY_AIRCON1_VS_DOWN    0x28
#define AIRWAY_AIRCON1_VS_SWING   0x38

class AIRWAYHeatpumpIR : public HeatpumpIR
{
  public:
    AIRWAYHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendAIRWAY(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
