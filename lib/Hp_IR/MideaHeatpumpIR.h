/*
    Midea MSR1-12HRN1-QC2 + MOA1-12HN1-QC2 heatpump control (remote control P/N RG51M1/E)
    This heatpump is sold as 'Ultimate Pro Plus 13FP' in Finland, by www.ultimatemarket.com

    Midea MSR1U-12HRDN1-QRC4W + MOB-12HFN1-QRC4W (remote control P/N RG51I20/BGE)
    This heatpump is sold as 'Ultimate 12 Pro Plus Inverter' in Finland, by www.ultimatemarket.com
*/
#ifndef MideaHeatpumpIR_h
#define MideaHeatpumpIR_h

#include <HeatpumpIR.h>

// Midea timing constants
#define MIDEA_AIRCON1_HDR_MARK       4420
#define MIDEA_AIRCON1_HDR_SPACE      4300
#define MIDEA_AIRCON1_BIT_MARK       620
#define MIDEA_AIRCON1_ONE_SPACE      1560
#define MIDEA_AIRCON1_ZERO_SPACE     480
#define MIDEA_AIRCON1_MSG_SPACE      5100

// MIDEA codes
#define MIDEA_AIRCON1_MODE_AUTO      0x10 // Operating mode
#define MIDEA_AIRCON1_MODE_HEAT      0x30
#define MIDEA_AIRCON1_MODE_COOL      0x00
#define MIDEA_AIRCON1_MODE_DRY       0x20
#define MIDEA_AIRCON1_MODE_FAN       0x60
#define MIDEA_AIRCON1_MODE_FP        0x70 // Not a real mode...
#define MIDEA_AIRCON1_MODE_OFF       0xFE // Power OFF - not real codes, but we need something...
#define MIDEA_AIRCON1_MODE_ON        0xFF // Power ON
#define MIDEA_AIRCON1_FAN_AUTO       0x02 // Fan speed
#define MIDEA_AIRCON1_FAN1           0x06
#define MIDEA_AIRCON1_FAN2           0x05
#define MIDEA_AIRCON1_FAN3           0x03


class MideaHeatpumpIR : public HeatpumpIR
{
  public:
    MideaHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendMidea(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature);
    void sendMidearaw(IRSender& IR, uint8_t sendBuffer[]);
};

#endif
