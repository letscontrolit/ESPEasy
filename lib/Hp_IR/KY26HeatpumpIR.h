// KY-26 Remote protocol
//
// The KY-26 remote control is used in locally branded air conditioners.
// I used a ZAICON air conditioner, which also seems to be rebranded as
// SACOM, SENCYS, and possibly others.
//
// The remote sends a 4-byte message which contains all possible settings every
// time.
//
// Byte 0 contains the a power signal, operating mode, and fan speed.
// Byte 1 contains the temperature setting.
// Byte 2 contains the timer setting.
// Byte 3 contains the checksum.
//
// MSB                                    LSB
// ??FF PPOO  ??TT TTTT  ???t tttt  CCCC CCCC
// 0000 0000  0000 0000  0000 0000  0000 0000
//   |  | |     || |        |       |
//   |  | |     || |        |       +--- Checksum
//   |  | |     || |        |
//   |  | |     || |        +--- Temperature (15 = 0, 16 = 16, ..., 31 = 31)
//   |  | |     || |
//   |  | |     || +--- Timer hours (0 = off, 1 = 1 hour, ..., 12 = 12 hours)
//   |  | |     |+----- Timer half hour
//   |  | |     +------ Timer on
//   |  | |
//   |  | +--- Operating mode (0 auto, 1 cool, 2 dry, 3 fan)
//   |  +----- Power (2 Toggle, 3 Turn off)
//   +-------- Fan speed (1 min, 2 mid, 3 max)
//

#ifndef KY26HeatpumpIR_h
#define KY26HeatpumpIR_h

#include <HeatpumpIR.h>

// Timing constants
#define KY26_HDR_MARK 9600
#define KY26_HDR_SPACE 6100
#define KY26_BIT_MARK 500
#define KY26_ONE_SPACE 1700
#define KY26_ZERO_SPACE 500

// Power modes
#define KY26_POWER_ONOFF 0x08
#define KY26_POWER_OFF 0x0C

// Operating modes
#define KY26_MODE_AUTO 0x00
#define KY26_MODE_COOL 0x01
#define KY26_MODE_DRY 0x02
#define KY26_MODE_FAN 0x03

// Fan speeds
#define KY26_FAN1 0x10 // * low
#define KY26_FAN2 0x20 // * med
#define KY26_FAN3 0x30 // * high

// Timer
#define KY26_TIMER_ON 0x20
#define KY26_TIMER_OFF 0x00
#define KY26_TIMER_HALF_HOUR 0x10

class KY26HeatpumpIR : public HeatpumpIR {
public:
  KY26HeatpumpIR();
  using HeatpumpIR::send;
  void send(IRSender &IR, uint8_t powerModeCmd, uint8_t operatingModeCmd,
            uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd,
            uint8_t swingHCmd, uint8_t timerHourCmd = 0x00,
            bool timerHalfHourCmd = false);

protected:
  void sendKY26(IRSender &IR, uint8_t powerModeCmd, uint8_t operatingModeCmd,
                uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t timerCmd);
};

#endif // KY26HeatpumpIR_h