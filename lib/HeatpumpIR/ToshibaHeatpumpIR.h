/*
    Toshiba RAS-10PKVP-ND heatpump control (remote control P/N WH-H07JE)
*/
#ifndef ToshibaHeatpumpIR_h
#define ToshibaHeatpumpIR_h

#include <HeatpumpIR.h>


//Toshiba timing constants
#define TOSHIBA_AIRCON1_HDR_MARK   4400
#define TOSHIBA_AIRCON1_HDR_SPACE  4400
#define TOSHIBA_AIRCON1_BIT_MARK   550
#define TOSHIBA_AIRCON1_ONE_SPACE  1600
#define TOSHIBA_AIRCON1_ZERO_SPACE 550


// Toshiba codes
#define TOSHIBA_AIRCON1_MODE_AUTO  0x00 // Operating mode
#define TOSHIBA_AIRCON1_MODE_HEAT  0xC0
#define TOSHIBA_AIRCON1_MODE_COOL  0x80
#define TOSHIBA_AIRCON1_MODE_DRY   0x40
#define TOSHIBA_AIRCON1_MODE_OFF   0xE0
#define TOSHIBA_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define TOSHIBA_AIRCON1_FAN1       0x02
#define TOSHIBA_AIRCON1_FAN2       0x06
#define TOSHIBA_AIRCON1_FAN3       0x01
#define TOSHIBA_AIRCON1_FAN4       0x05
#define TOSHIBA_AIRCON1_FAN5       0x03

class ToshibaHeatpumpIR : public HeatpumpIR
{
  public:
    ToshibaHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendToshiba(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
