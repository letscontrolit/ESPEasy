/*
    Ballu heatpump control
 */
#ifndef BalluHeatpumpIR_h
#define BalluHeatpumpIR_h

#include <HeatpumpIR.h>

// BALLU timing constants
#define BALLU_AIRCON_HDR_MARK   9300  //
#define BALLU_AIRCON_HDR_SPACE  4550  //
#define BALLU_AIRCON_BIT_MARK   500   //
#define BALLU_AIRCON_ZERO_SPACE 500   //
#define BALLU_AIRCON_ONE_SPACE  1650  //
#define BALLU_AIRCON_MSG_SPACE  0     //

// BALLU codes
//#define BALLU_AIRCON_MODE_AUTO  0x00 // Operating mode
#define BALLU_AIRCON_MODE_HEAT  0x00
#define BALLU_AIRCON_MODE_COOL  0x02
#define BALLU_AIRCON_MODE_DRY   0x03
#define BALLU_AIRCON_MODE_FAN   0x04

#define BALLU_AIRCON_MODE_OFF   0x04 // Power OFF
#define BALLU_AIRCON_MODE_ON    0x00

#define BALLU_AIRCON_FAN_AUTO   0x00 // Fan speed
#define BALLU_AIRCON_FAN1       0x01 // max
#define BALLU_AIRCON_FAN2       0x02
#define BALLU_AIRCON_FAN3       0x03 // min



class BalluHeatpumpIR : public HeatpumpIR
{
  public:
    BalluHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd);

  private:
    void sendBallu(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature);
};

#endif
