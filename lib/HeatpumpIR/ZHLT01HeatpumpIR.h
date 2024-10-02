/******************************************************************************** 
 *  Airconditional remote control encoder for:
 *  
 *  ZH/LT-01 Remote control https://www.google.com/search?q=zh/lt-01
 *  
 *  The ZH/LT-01 remote control is used for many locally branded Split 
 *  airconditioners, so it is better to name this protocol by the name of the 
 *  REMOTE rather then the name of the Airconditioner. For this project I used 
 *  a 2014 model Eurom-airconditioner, which is Dutch-branded and sold in 
 *  the Netherlands at Hornbach.
 *
 * For airco-brands: 
 *  Eurom
 *  Chigo
 *  Tristar
 *  Tecnomaster
 *  Elgin
 *  Geant
 *  Tekno
 *  Topair
 *  Proma
 *  Sumikura
 *  JBS
 *  Turbo Air
 *  Nakatomy
 *  Celestial Air
 *  Ager
 *  Blueway
 *  Airlux
 *  Etc.
 *  
 ***********************************************************************************  
 *  SUMMARY FUNCTIONAL DESCRIPTION
 **********************************************************************************
 *  The remote sends a 12 Byte message which contains all possible settings every
 *  time.
 *  
 *  Byte 11 (and 10) contain the remote control identifier and are always 0xD5 and
 *  0x2A respectively for the ZH/LT-01 remote control. 
 *  Every UNeven Byte (01,03,05,07 and 09) holds command data
 *  Every EVEN Byte (00,02,04,06,08 and 10) holds a checksum of the corresponding 
 *  command-, or identifier-byte by _inverting_ the bits, for example:
 *  
 *  The identifier byte[11] = 0xD5 = B1101 0101
 *  The checksum byte[10]   = 0x2A = B0010 1010
 *  
 *  So, you can check the message by:
 *  - inverting the bits of the checksum byte with the corresponding command-, or 
 *    identifier byte, they should me the same, or
 *  - Summing up the checksum byte and the corresponding command-, or identifier byte,
 *    they should always add up to 0xFF = B11111111 = 255
 *  
 *  Control bytes:
 *  [01] - Timer (1-24 hours, Off)
 *         Time is hardcoded to OFF
 *
 *  [03] - LAMP ON/OFF, TURBO ON/OFF, HOLD ON/OFF 
 *         Lamp and Hold are hardcoded to OFF
 *         Turbo is used for Cool and Heat is case of FAN_4 or FAN_5
 *
 *  [05] - Indicates which button the user _pressed_ on the remote control
 *         Hardcoded to POWER-button
 *
 *  [07] - POWER ON/OFF, FAN AUTO/3/2/1, SLEEP ON/OFF, AIRFLOW ON/OFF,
 *         VERTICAL SWING/WIND/FIXED
 *         SLEEP is used for FAN_SILENT
 *         FAN_4 and FAN_5 are not supported, coded to FAN_3 or Turbo (byte [3])
 *         Vertical Swing supports Fixed, Swing and "Wind"
 *            VDIR_AUTO  = Wind
 *            VDIR_SWING = Swing
 *            All others = Fixed
 *
 *  [09] - MODE AUTO/COOL/VENT/DRY/HEAT, TEMPERATURE (16 - 32°C)
 *         MODE_MAINT is not supported, but implemented as MODE_DRY at 30°C
 *  
 * ****************************************************************************** 
 *  Written by: Marcel van der Kooij 
 *  Date: 2020-10-29
 *  Version: 1.0
 *******************************************************************************/
 
#ifndef ZHLT01HeatpumpIR_h
#define ZHLT01HeatpumpIR_h

#include <HeatpumpIR.h>

/********************************************************************************
 *  TIMINGS 
 *  Space:        Not used
 *  Header Mark:  6200 us
 *  Header Space: 7500 us
 *  Bit Mark:      475 us
 *  Zero Space:    525 us
 *  One Space:    1650 us
 *******************************************************************************/
#define AC1_HDR_MARK   6200
#define AC1_HDR_SPACE  7500
#define AC1_BIT_MARK    475 
#define AC1_ZERO_SPACE  525 
#define AC1_ONE_SPACE  1650

/********************************************************************************
 * 
 * ZHLT01 codes
 * 
 *******************************************************************************/

// Power
#define AC1_POWER_OFF  0x00
#define AC1_POWER_ON   0x02

// Operating Modes
#define AC1_MODE_AUTO  0x00
#define AC1_MODE_COOL  0x20
#define AC1_MODE_DRY   0x40
#define AC1_MODE_FAN   0x60
#define AC1_MODE_HEAT  0x80
// MODE_MAINT is not supported, but implemented as DRY at 30°C.

//Fan control
#define AC1_FAN_AUTO   0x00
#define AC1_FAN_SILENT 0x01
#define AC1_FAN1       0x60
#define AC1_FAN2       0x40
#define AC1_FAN3       0x20
#define AC1_FAN_TURBO  0x08  
// FAN_4 and FAN_5 are not supported, but are implemented as button "TURBO"
// This only works for HEAT and COOL. Otherwise FAN_3 is used.

// Vertical Swing
#define AC1_VDIR_WIND   0x00  // "Natural Wind", implemented on VDIR_AUTO
#define AC1_VDIR_SWING  0x04  // Swing
#define AC1_VDIR_FIXED  0x08  // All others are not supported 
                                        // and implemented as Fixed

// Horizontal Swing
#define AC1_HDIR_SWING  0x00  // HDIR_SWING
#define AC1_HDIR_FIXED  0x10  // All others are not supported 
                                        // and implemented as Fixed

class ZHLT01HeatpumpIR : public HeatpumpIR
{
  public:
    ZHLT01HeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, 
	                        uint8_t operatingModeCmd, 
							uint8_t fanSpeedCmd, 
							uint8_t temperatureCmd, 
							uint8_t swingVCmd, 
							uint8_t swingHCmd);

  protected:
    void sendZHLT01(IRSender& IR, uint8_t powerMode, 
	                             uint8_t operatingMode, 
								 uint8_t fanSpeed, 
								 uint8_t temperature,
								 uint8_t swingV,
								 uint8_t swingH);
};

#endif
