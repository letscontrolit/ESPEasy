/*
Info about NIBE IR remote code:

Control Message:
Consists of 90 Bits
Message is split into
| 10x8 Data Bits | 2 Static Bit (01) | CRC 8 bit |

Data Bits are split up the following way:
| Never Change | Never Change | Op Mode and Temp | Temp and Fan Mode | Vertical Direction and Timer | Timer | Timer | Timer and Time | Time and Special Functions | Special Functions |

iFeel Message:
Consists of 32 Bits
| 3x8 Data Bits | CRC 8 bit|

Last 5 Bits of the Data Bits contain the sensed temperature
from 4-35 deg C. Temperature is transmitted with an offset of 4 deg.
Data are being send every 5 min or when the sensed temperature value changes.
*/

#include <stdint.h>
#include "NibeHeatpumpIR.h"

#ifdef NIBE_USE_TIME_H
#include <time.h>
#endif

#ifdef NIBE_IR_SEND_TIME
int nibeSendHour = 12;
int nibeSendMinute = 42;
#endif

// Reverses bit order for a uint8_t. Can modify the bitlength that needs to be reversed (needed 8, 5 bit or 2 bit reverse)
static uint8_t reverseBits8(uint8_t value, int bitLength) {
    uint8_t reversedValue = 0;
    
    for (int i = 0; i < bitLength; i++) {
        // Extract the i-th bit from the original value
        uint8_t bit = (value >> i) & 1;
        
        // Set the (bitLength - 1 - i)-th bit in the reversed value
        reversedValue |= bit << (bitLength - 1 - i);
    }

    return reversedValue;
}

// Reverses bit order for a uint16_t. Can modify the bitlength that needs to be reversed (needed 11 bit reverse)
static uint16_t reverseBits16(uint16_t value, int bitLength) {
    uint16_t reversedValue = 0;
    
    for (int i = 0; i < bitLength; i++) {
        // Extract the i-th bit from the original value
        uint16_t bit = (value >> i) & 1;
        
        // Set the (bitLength - 1 - i)-th bit in the reversed value
        reversedValue |= bit << (bitLength - 1 - i);
    }
    
    return reversedValue;
}

NibeHeatpumpIR::NibeHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "nibe";
  static const char info[]  PROGMEM = "{\"mdl\":\"nibe\",\"dn\":\"Nibe\",\"mT\":10,\"xT\":32,\"fs\":5}";

  _model = model;
  _info = info;
}

void NibeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, false, false);
}

void NibeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboModeCmd, bool iFeelModeCmd)
{
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode
  uint8_t powerMode = NIBE_POWER_ON;
  uint8_t operatingMode = NIBE_MODE_HEAT_ONDEMAND;  // Default run unit until temp is reached, fan stops
  uint8_t fanSpeed = NIBE_MODE_FAN_AUTO;
  uint8_t temperature = 21;
  uint8_t swingV = NIBE_VDIR_AUTO;  // Set auto mode since that one is allowed for all modes
  uint8_t iFeelMode = 0x00;
  uint8_t filter = 0x00;
  uint8_t nightMode = 0x00;
  uint8_t turboMode = 0x00;
  bool filterCmd = true;  // Default enable air filter

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = NIBE_POWER_OFF;
  }
  else
  {
    powerMode = NIBE_POWER_ON;

    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = NIBE_MODE_AUTO_HEAT;
        //operatingMode = NIBE_MODE_AUTO_COOL;  // This heatpump can be set from heating or cooling into auto mode. Effect is the same.
        break;
      case MODE_HEAT:
        operatingMode = NIBE_MODE_HEAT_ONDEMAND;    // Heats until temp is reached, turns off fan when target is reached
        //operatingMode = NIBE_MODE_HEAT_CONTINOUS; // Heats until temp is reached, but keeps the fan running 
        break;
      case MODE_COOL:
        operatingMode = NIBE_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = NIBE_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = NIBE_MODE_FAN;
        break;
    }
  }

  // NOTE Fan speed Auto can not be used in MODE_FAN
  // TODO not sure this fan mapping is correct? FAN_1 = low speed and FAN_3 = high speed?
  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      if (operatingMode == NIBE_MODE_FAN)
        fanSpeed = NIBE_MODE_FAN1;
      else
        fanSpeed = NIBE_MODE_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = NIBE_MODE_FAN3;
      break;
    case FAN_2:
      fanSpeed = NIBE_MODE_FAN2;
      break;
    case FAN_3:
      fanSpeed = NIBE_MODE_FAN1;
      break;
    case FAN_SILENT:
      nightMode = 0x01;
      break;
  }

// NOTE Operating Mode Auto allows all positions
// NOTE Operating Mode Cooling and Dry allows only pos 1-4
// NOTE Operating Mode Heating allows only pos 3-6
// NOTE Position 6 is not implemented!
  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = NIBE_VDIR_AUTO;
      break;
    case VDIR_SWING:
      swingV = NIBE_VDIR_ALL;
      break;
    case VDIR_UP:
      if ((operatingMode == NIBE_MODE_HEAT_CONTINOUS) || (operatingMode == NIBE_MODE_HEAT_ONDEMAND))
        swingV = NIBE_VDIR_POS3;
      else
        swingV = NIBE_VDIR_POS1;
      break;
    case VDIR_MUP:
      if ((operatingMode == NIBE_MODE_HEAT_CONTINOUS) || (operatingMode == NIBE_MODE_HEAT_ONDEMAND))
        swingV = NIBE_VDIR_POS3;
      else
        swingV = NIBE_VDIR_POS2;
      break;
    case VDIR_MIDDLE:
      swingV = NIBE_VDIR_POS3;
      break;
    case VDIR_MDOWN:
      swingV = NIBE_VDIR_POS4;
      break;
    case VDIR_DOWN:
      if ((operatingMode == NIBE_MODE_COOL) || (operatingMode == NIBE_MODE_DRY))
        swingV = NIBE_VDIR_POS4;
      else
        swingV = NIBE_VDIR_POS5;
      break;
  }

  if (iFeelModeCmd)
  {
    iFeelMode = 0x01;
  }

  if (filterCmd)
  {
    filter = 0x01;
  }

  if (turboModeCmd)
  {
    turboMode = 0x01;
  }

  // Allowed temp range 10-32 deg
  if ((temperatureCmd > 9) && (temperatureCmd < 33))
  {
    temperature = temperatureCmd - 4; // Temp is reported as: Actual Temp - 4
  }

  sendNibe(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, iFeelMode, filter, turboMode, nightMode);
}

// Send the Gree code
void NibeHeatpumpIR::sendNibe(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t iFeelMode, uint8_t filter, uint8_t turboMode, uint8_t nightMode)
{
  // Setting some default values that never change!
  uint8_t NibeTemplate[] = { 0x35, 0xAF, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };
  //                            0     1     2     3     4     5     6     7     8     9    10    11

#ifdef NIBE_USE_TIME_H
  time_t now;
  struct tm * timeinfo;
#endif
  uint16_t currentTime = 0;  // If no USE_TIME_H or IR_SEND_TIME defined, send always time 00:00

  // Convert Temperature
  uint8_t temp_swap = reverseBits8(temperature, 5);

  // Byte 2 contains Operation Mode and part of temperature
  NibeTemplate[2] |= operatingMode << 2;
  NibeTemplate[2] |= (temp_swap >> 3);

  // Byte 3 contains part of temperature and fan mode
  NibeTemplate[3] |= (temp_swap & 0x07) << 5;
  NibeTemplate[3] |= fanSpeed << 3;

  // NOTE Part of Byte 3 and 4 (total of 5 bits) contains some data that are not decoded. They seem to change randomly.
  // Same commands have been send with the remote control, while part of these 5 bits are changing.
  // Observed Codes:
  // 00001, 10001, 01001, 00101, 10101, 01101
  // We always use 00001 in the hope that it doesnt affect the usage of the heatpump! Maybe someone will figure it out when to change these!
  // During testing no issues have been observed!

  // Byte 4 - vertical air flow direction
  NibeTemplate[4] |= swingV << 3;

  // NOTE Timer functionality is not implemented here, when disabled timer fields always have the same value
  // Part of Byte 4 and Byte 5 is for start timer function -> needs to be set to 00000000111
  NibeTemplate[5] |= 0x7;
  // Byte 6 and part of Byte 7 is for stop timer function -> needs to be set to 00000000111
  NibeTemplate[7] |= (0x7 << 5);
  // Part of Byte 7 and part of Byte 8 is for actual time (needed for timer and vacation function)
#ifdef NIBE_USE_TIME_H
  time(&now);
  timeinfo = localtime(&now);
  currentTime = (uint16_t)(timeinfo->tm_hour * 60 + timeinfo->tm_min);
#endif
#ifdef NIBE_IR_SEND_TIME
  currentTime = (uint16_t)(nibeSendHour * 60 + nibeSendMinute);
#endif
  currentTime = reverseBits16(currentTime, 11);
  NibeTemplate[7] |= ((uint8_t) ((currentTime & 0x7C0) >> 6));
  NibeTemplate[8] |= ((uint8_t) (currentTime & 0x3F)) << 2;

  // Part of Byte 8 and part of Byte 9 is for special functions
  NibeTemplate[9] |= iFeelMode;
  NibeTemplate[9] |= (powerMode << 2);
  NibeTemplate[9] |= (filter << 3);
  NibeTemplate[9] |= (turboMode << 4);
  NibeTemplate[9] |= (nightMode << 5);
  
  // Calculate CRC
  uint8_t checksum = 0;
  for (int i = 0; i < 11; i++)
  {
    if (i == 10)
      checksum += reverseBits8(NibeTemplate[i], 2);  // Byte 10 only has 2 bits
    else
      checksum += reverseBits8(NibeTemplate[i], 8);
  }
  NibeTemplate[11] = reverseBits8(checksum, 8);

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(NIBE_HDR_MARK);
  IR.space(NIBE_HDR_SPACE);

  for (int i=0; i<12; i++) 
  {
    if (i == 10)
      IR.sendIRbyte(reverseBits8(NibeTemplate[i],2), NIBE_BIT_MARK, NIBE_ZERO_SPACE, NIBE_ONE_SPACE, 2); // Byte 10 only has 2 bits
    else
      IR.sendIRbyte(reverseBits8(NibeTemplate[i],8), NIBE_BIT_MARK, NIBE_ZERO_SPACE, NIBE_ONE_SPACE);
  }

  // End mark
  IR.mark(NIBE_BIT_MARK);
  IR.space(NIBE_MSG_SPACE);
}

//Should send current sensed temperatures every 5 min or on temperature change
void NibeHeatpumpIR::send(IRSender& IR, uint8_t currentTemperature)
{
  // Example: 0011 0101 1010 1111 0100 0101 1100 0010
  uint8_t NibeTemplate[] = { 0x35, 0xAF, 0x40, 0x00 };

  uint8_t temp_swap = reverseBits8(currentTemperature - 4, 5);

  NibeTemplate[2] |= temp_swap;

  uint8_t checksum = 0;
  for (int i = 0; i < 3; i++) 
  {
    checksum += reverseBits8(NibeTemplate[i], 8);
  }

  NibeTemplate[3] = reverseBits8(checksum, 8);

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(NIBE_HDR_MARK);
  IR.space(NIBE_HDR_SPACE);

  for (int i=0; i<4; i++) 
  {
    IR.sendIRbyte(reverseBits8(NibeTemplate[i], 8), NIBE_BIT_MARK, NIBE_ZERO_SPACE, NIBE_ONE_SPACE);
  }

  // End mark
  IR.mark(NIBE_BIT_MARK);
  IR.space(NIBE_MSG_SPACE);
}
