/********************************************************************************
 *  Airconditional remote control decoder for:
 *
 *  ZH/JG-01 Remote control https://www.google.com/search?q=zh/JG-01
 *
 *  The ZH/JG-01 remote control is used for many locally branded Split airconditioners,
 *  so it is better to name this protocol by the name of the protocol rather then the
 *  name of the Airconditioner. For this project I used a TACHIAIR airconditioner.
 *
 * For airco-brands:
 *  Tachiair
 *  Chigo
 *  Etc.
 *
 ***********************************************************************************
 *  SUMMARY FUNCTIONAL DESCRIPTION
 **********************************************************************************
 *  The remote sends a 6 Byte message which contains all possible settings every
 *  time.
 *
 *  Every EVEN Byte (00,02,04,06,08 and 10) hold command data
 *  Every UNeven Byte (01,03,05,07 and 09)  hold a checksum of the corresponding
 *  command by inverting the bits, for example:
 *
 *  The identifier byte[0] = 0xD5 = B1101 0101
 *  The checksum byte[1]   = 0x2A = B0010 1010
 *
 *  So, you can check the message by:
 *  - inverting the bits of the checksum byte with the corresponding command, they
 *    should be the same, or
 *  - Summing up the checksum byte and the corresponding command,
 *    they should always add up to 0xFF = B11111111 = 255
 *
 * ******************************************************************************
 *  Written by: Abílio Costa
 *  Date: 2023-07-03
 *  Version: 1.0
 *******************************************************************************/

#ifndef ZHJG01HeatpumpIR_h
#define ZHJG01HeatpumpIR_h

#include <HeatpumpIR.h>

/********************************************************************************
 *  TIMINGS
 *  Space:        Not used
 *  Header Mark:  6550 us
 *  Header Space: 7755 us
 *  Bit Mark:     560 us
 *  Zero Space:   1530 us
 *  One Space:    3750 us
 *******************************************************************************/
#define ZHJG01_HDR_MARK   6550
#define ZHJG01_HDR_SPACE  7755
#define ZHJG01_BIT_MARK    560
#define ZHJG01_ZERO_SPACE 1530
#define ZHJG01_ONE_SPACE  3750

/********************************************************************************
 *
 * ZHJG01 codes
 *
 *******************************************************************************/

// Power
#define ZHJG01_POWER_OFF  0x00
#define ZHJG01_POWER_ON   0x08

// Operating Modes
#define ZHJG01_MODE_AUTO  0x00
#define ZHJG01_MODE_COOL  0x01
#define ZHJG01_MODE_DRY   0x02
#define ZHJG01_MODE_FAN   0x03
#define ZHJG01_MODE_HEAT  0x04

//Fan control
#define ZHJG01_FAN_AUTO   0x00
#define ZHJG01_FAN1       0x60
#define ZHJG01_FAN2       0x40
#define ZHJG01_FAN3       0x20
#define ZHJG01_FAN_TURBO  0x80
#define ZHJG01_FAN_ECO    0xA0
// FAN_4 and FAN_5 are not supported, but are implemented as button "TURBO"
// This only works for HEAT and COOL. Otherwise FAN_3 is used.
// FAN_SILENT is not supported, but is implmented as button "ECO".

// Vertical Swing
#define ZHJG01_VDIR_WIND   0x00  // "Natural Wind", implemented on VDIR_AUTO
#define ZHJG01_VDIR_SWING  0x08  // Swing
#define ZHJG01_VDIR_FIXED  0x10  // All others are not supported
                                 // and implemented as Fixed

class ZHJG01HeatpumpIR : public HeatpumpIR
{
  public:
    ZHJG01HeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd,
                            uint8_t operatingModeCmd,
                            uint8_t fanSpeedCmd,
                            uint8_t temperatureCmd,
                            uint8_t swingVCmd,
                            uint8_t swingHCmd);

  protected:
    void sendZHJG01(IRSender& IR, uint8_t powerMode,
                                  uint8_t operatingMode,
                                  uint8_t fanSpeed,
                                  uint8_t temperature,
                                  uint8_t swingV);
};

#endif
