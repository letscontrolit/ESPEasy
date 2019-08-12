/*
    Toshiba Daiseikai (RAS-10G2KVP-E RAS-10G2AVP-E and RAS-13G2KVP-E RAS-13G2AVP-E)
*/
#ifndef ToshibaDaiseikaiHeatpumpIR_h
#define ToshibaDaiseikaiHeatpumpIR_h

#include <HeatpumpIR.h>
#include <CarrierHeatpumpIR.h> // Toshiba Daiseikai is based on the Carrier models


// Toshiba Daiseikai (RAS-10G2KVP-E RAS-10G2AVP-E and RAS-13G2KVP-E RAS-13G2AVP-E) timing constants (remote control P/N WH-TA01EE)
// https://github.com/ToniA/arduino-heatpumpir/issues/23
#define DAISEIKAI_AIRCON1_HDR_MARK   4320
#define DAISEIKAI_AIRCON1_HDR_SPACE  4350
#define DAISEIKAI_AIRCON1_BIT_MARK   500
#define DAISEIKAI_AIRCON1_ONE_SPACE  1650
#define DAISEIKAI_AIRCON1_ZERO_SPACE 550
#define DAISEIKAI_AIRCON1_MSG_SPACE  7400

// Toshiba Daiseikai (Carrier) codes. Same as CarrierNQV ?
#define DAISEIKAI_AIRCON1_MODE_AUTO  0x00 // Operating mode
#define DAISEIKAI_AIRCON1_MODE_HEAT  0xC0
#define DAISEIKAI_AIRCON1_MODE_COOL  0x80
#define DAISEIKAI_AIRCON1_MODE_DRY   0x40
#define DAISEIKAI_AIRCON1_MODE_FAN   0x20
#define DAISEIKAI_AIRCON1_MODE_OFF   0xE0 // Power OFF
#define DAISEIKAI_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define DAISEIKAI_AIRCON1_FAN1       0x02
#define DAISEIKAI_AIRCON1_FAN2       0x06
#define DAISEIKAI_AIRCON1_FAN3       0x01
#define DAISEIKAI_AIRCON1_FAN4       0x05
#define DAISEIKAI_AIRCON1_FAN5       0x03


class ToshibaDaiseikaiHeatpumpIR : public CarrierHeatpumpIR
{
  public:
    ToshibaDaiseikaiHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendDaiseikai(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature);
};


#endif
