/*
    Hyundai heatpump control (remote control P/N Y512F2)
*/
#ifndef HyundaiHeatpumpIR_h
#define HyundaiHeatpumpIR_h

#include <HeatpumpIR.h>


// Hyundai timing constants
#define HYUNDAI_AIRCON1_HDR_MARK   8840 // 8700
#define HYUNDAI_AIRCON1_HDR_SPACE  4440 // 4200
#define HYUNDAI_AIRCON1_BIT_MARK   640  // 580
#define HYUNDAI_AIRCON1_ONE_SPACE  1670 // 1530
#define HYUNDAI_AIRCON1_ZERO_SPACE 570  // 460

// Hyundai codes
#define HYUNDAI_AIRCON1_MODE_AUTO  0x00 // Operating mode
#define HYUNDAI_AIRCON1_MODE_HEAT  0x04
#define HYUNDAI_AIRCON1_MODE_COOL  0x01
#define HYUNDAI_AIRCON1_MODE_DRY   0x02
#define HYUNDAI_AIRCON1_MODE_FAN   0x03
#define HYUNDAI_AIRCON1_MODE_OFF   0x00 // Power OFF
#define HYUNDAI_AIRCON1_MODE_ON    0x08 // Power ON
#define HYUNDAI_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define HYUNDAI_AIRCON1_FAN1       0x10
#define HYUNDAI_AIRCON1_FAN2       0x20
#define HYUNDAI_AIRCON1_FAN3       0x30

#define HYUNDAI_AIRCON1_VS_SWING   0x40 // Vertical swing
#define HYUNDAI_AIRCON1_VS_AUTO    0x00 // Automatic setting


class HyundaiHeatpumpIR : public HeatpumpIR
{
  public:
    HyundaiHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendHyundai(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingVCmd, uint8_t swingHCmd);
};

#endif
