/*
  Nibe heatpump control (Tested for Nibe Model AG-WL10-4)
*/
#ifndef NibeHeatpumpIR_h
#define NibeHeatpumpIR_h

#include <HeatpumpIR.h>

#define NIBE_USE_TIME_H
//#define NIBE_IR_SEND_TIME

#define NIBE_HDR_MARK   6382
#define NIBE_HDR_SPACE  3144
#define NIBE_BIT_MARK   412
#define NIBE_ONE_SPACE  2102
#define NIBE_ZERO_SPACE 823
#define NIBE_MSG_SPACE  0

// Power state (1 bit)
#define NIBE_POWER_OFF  0x00
#define NIBE_POWER_ON   0x01

// Operating modes
// Nibe codes (combination of 3 bits)
#define NIBE_MODE_COOL           0x00
#define NIBE_MODE_HEAT_ONDEMAND  0x01
#define NIBE_MODE_HEAT_CONTINOUS 0x03
#define NIBE_MODE_DRY            0x04
#define NIBE_MODE_FAN            0x06
// Auto mode can be achieved via cool or heating button. They create different codes, but should do same function
#define NIBE_MODE_AUTO_HEAT      0x05
#define NIBE_MODE_AUTO_COOL      0x02

// Fan speeds (2 bits)
//NOTE Fan speed Auto can not be used in MODE_FAN
#define NIBE_MODE_FAN_AUTO   0x00 // Fan speed
#define NIBE_MODE_FAN1       0x03 // * Max
#define NIBE_MODE_FAN2       0x01 // * Medium
#define NIBE_MODE_FAN3       0x02 // * Low

// Vertical air directions (3 bits)
// Note according to the remote control manual there are limitations:
// Operating Mode Auto allows all positions
// Operating Mode Cooling and Dry allows only pos 1-4
// Operating Mode Heating allows only pos 3-6
// When testing all modes accepted the different inputs.
#define NIBE_VDIR_AUTO  0x00
#define NIBE_VDIR_POS1  0x04
#define NIBE_VDIR_POS2  0x02
#define NIBE_VDIR_POS3  0x06
#define NIBE_VDIR_POS4  0x01
#define NIBE_VDIR_POS5  0x05
#define NIBE_VDIR_POS6  0x03
#define NIBE_VDIR_ALL   0x07

//NOTE Horizontal air direction can be changed manually on the unit

// Special Functions:
// The Remote Control comes with 4 special features
// iFeel -> Uses remote control to transmit temperature
// Night Program -> changes temperature and fan speed, 1h after it has been activated
// Filter -> Enable Air Ioniser, which effectively prevents bad odours and eliminates bacteria and microorganisms.
// Turbo Mode -> Full power Heating or Cooling

// NOTE There are more functions such as timer and vacation mode, but they are not being used here!

class NibeHeatpumpIR : public HeatpumpIR
{
  public:
    NibeHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboModeCmd, bool iFeelModeCmd);
    void send(IRSender& IR, uint8_t currentTemperature);

  private:
    void sendNibe(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t iFeelMode, uint8_t filter, uint8_t turboMode, uint8_t nightMode);
};

#ifdef NIBE_IR_SEND_TIME
extern int nibeSendHour;
extern int nibeSendMinute;
#endif

#endif
