/*
    Fuego etc. (Vivax, Classe, NEO, Galanz, Simbio, Beko, Sintech) heatpump control (remote control P/N GZ-1002B-E3)
*/
#ifndef FuegoHeatpumpIR_h
#define FuegoHeatpumpIR_h

#include <HeatpumpIR.h>


//Fuego timing constants
#define FUEGO_AIRCON1_HDR_MARK   3600
#define FUEGO_AIRCON1_HDR_SPACE  1630
#define FUEGO_AIRCON1_BIT_MARK   420
#define FUEGO_AIRCON1_ONE_SPACE  1380
#define FUEGO_AIRCON1_ZERO_SPACE 420

// Fuego codes
#define FUEGO_AIRCON1_MODE_AUTO  0x08 // Operating mode
#define FUEGO_AIRCON1_MODE_HEAT  0x01
#define FUEGO_AIRCON1_MODE_COOL  0x03
#define FUEGO_AIRCON1_MODE_DRY   0x02
#define FUEGO_AIRCON1_MODE_FAN   0x07
#define FUEGO_AIRCON1_MODE_ON    0x04 // Power ON
#define FUEGO_AIRCON1_MODE_OFF   0x00
#define FUEGO_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define FUEGO_AIRCON1_FAN1       0x02
#define FUEGO_AIRCON1_FAN2       0x03
#define FUEGO_AIRCON1_FAN3       0x05

#define FUEGO_AIRCON1_VS_AUTO    0x00 // Vertical swing
#define FUEGO_AIRCON1_VS_UP      0x08
#define FUEGO_AIRCON1_VS_MUP     0x10
#define FUEGO_AIRCON1_VS_MIDDLE  0x18
#define FUEGO_AIRCON1_VS_MDOWN   0x20
#define FUEGO_AIRCON1_VS_DOWN    0x28
#define FUEGO_AIRCON1_VS_SWING   0x38

class FuegoHeatpumpIR : public HeatpumpIR
{
  public:
    FuegoHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendFuego(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
