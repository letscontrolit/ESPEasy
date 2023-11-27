#include <ZHLT01HeatpumpIR.h>

ZHLT01HeatpumpIR::ZHLT01HeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "ZHLT01";
  static const char info[]  PROGMEM = "{\"mdl\":\"ZHLT01\",\"dn\":\"ZHLT01\",\"mT\":18,\"xT\":32,\"fs\":3,\"maint\":[10]}}";

  _model = model;
  _info = info;
}

void ZHLT01HeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, 
                                          uint8_t operatingModeCmd, 
			 							  uint8_t fanSpeedCmd, 
			 							  uint8_t temperatureCmd, 
			 							  uint8_t swingVCmd, 
			 							  uint8_t swingHCmd)
{

// Sensible defaults for the heat pump mode

  uint8_t powerMode     = AC1_POWER_ON;
  uint8_t operatingMode = AC1_MODE_AUTO;
  uint8_t fanSpeed      = AC1_FAN_AUTO;
  uint8_t temperature   = 25;
  uint8_t swingV        = AC1_VDIR_FIXED;
  uint8_t swingH        = AC1_HDIR_FIXED;

  if (powerModeCmd == 0)
  {
    powerMode = AC1_POWER_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_COOL:
      operatingMode  = AC1_MODE_COOL;
      break;
    case MODE_FAN:
      operatingMode  = AC1_MODE_FAN;
      break;
    case MODE_DRY:
      operatingMode  = AC1_MODE_DRY;
      temperatureCmd = 25;
      break;
    case MODE_HEAT:
      operatingMode  = AC1_MODE_HEAT;
      break;
    case MODE_MAINT:
      operatingMode  = AC1_MODE_DRY;       // MODE_MAINT not supported
      temperatureCmd = 30;                 // Simlulated as DRY at 30°C
      break;
	default:
      operatingMode  = AC1_MODE_AUTO;
	  temperatureCmd = 25;                 // "Auto" = 25°C
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = AC1_FAN_AUTO;
      break;
    case FAN_SILENT:
      fanSpeed = AC1_FAN_SILENT;
      break;
    case FAN_1:
      fanSpeed = AC1_FAN1;
      break;
    case FAN_2:
      fanSpeed = AC1_FAN2;
      break;
    case FAN_3:
      fanSpeed = AC1_FAN3;
      break;
    case FAN_4:
      if ( operatingMode == AC1_MODE_COOL || operatingMode == AC1_MODE_HEAT )
	  { fanSpeed = AC1_FAN_TURBO; }
      else
      { fanSpeed = AC1_FAN3; }
      break;
    case FAN_5:
      if ( operatingMode == AC1_MODE_COOL || operatingMode == AC1_MODE_HEAT )
	  { fanSpeed = AC1_FAN_TURBO; }
      else
      { fanSpeed = AC1_FAN3; }
      break;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = AC1_VDIR_WIND;
      break;
    case VDIR_SWING:
      swingV = AC1_VDIR_SWING;
      break;
	default:
      swingV = AC1_VDIR_FIXED;
  }

  switch (swingHCmd)
  {
    case HDIR_SWING:
      swingH = AC1_HDIR_SWING;
      break;
	default:
      swingH = AC1_HDIR_FIXED;  
  }

// temperature must be between 16 and 32 degrees

  temperature = temperatureCmd;
  
  if ( temperatureCmd < 16)
  {
    temperature = 16;
  }

  if (temperatureCmd > 32)
  {
    temperature = 32;
  }


  sendZHLT01(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

void ZHLT01HeatpumpIR::sendZHLT01(IRSender& IR, uint8_t powerMode, 
                                                uint8_t operatingMode, 
											    uint8_t fanSpeed, 
											    uint8_t temperature,
											    uint8_t swingV,
											    uint8_t swingH)
{
  uint8_t ZHLT01Template[] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x2A, 0xD5 };
  //    Bytenumbers:             0     1     2     3     4     5     6     7     8     9    10    11

/********************************************************************************
 * Byte[03]: Button TURBO used for fanspeed 4 and 5
 * TURBO ON: Bxxxx1xxx (bit3)
 * TURBO ON: Bxxxx0xxx
 *******************************************************************************/
  if (fanSpeed == AC1_FAN_TURBO)
  {
     ZHLT01Template[3] = AC1_FAN_TURBO;
     ZHLT01Template[2] = ~ ZHLT01Template[3];
	 fanSpeed          = AC1_FAN_AUTO;
  }

 /********************************************************************************
 * Byte[07]: POWER, FAN, SLEEP, HORIZONTAL, VERTICAL
 * POWER ON:         B0xxxxx1x
 * POWER OFF:        B0xxxxx0x
 * VERTICAL SWING:   B0xxx01xx
 * VERTICAL WIND:    B0xxx00xx
 * VERTICAL FIXED:   B0xxx10xx
 * HORIZONTAL SWING: B0xx0xxxx
 * HORIZONTAL OFF:   B0xx1xxxx
 * FAN AUTO:         B000xxxx0
 * FAN SILENT:       B000xxxx1
 * FAN3:             B001xxxx0
 * FAN2:             B010xxxx0
 * FAN1:             B011xxxx0
 *******************************************************************************/
  ZHLT01Template[7] = fanSpeed | powerMode | swingV | swingH;
  ZHLT01Template[6] = ~ ZHLT01Template[7];
  
/********************************************************************************
 * Byte[09]: Mode, Temperature
 * MODE AUTO: B000xxxxx
 * MODE COOL: B001xxxxx
 * MODE VENT: B011xxxxx
 * MODE DRY:  B010xxxxx
 * MODE HEAT: B100xxxxx
 * Temperature is determined by bit0-4:
 * 0x00 = 16C
 * 0x10 = 32C
 *******************************************************************************/

  ZHLT01Template[9] = operatingMode | (temperature - 16);
  ZHLT01Template[8] = ~ ZHLT01Template[9];

// 38 kHz PWM frequency
  IR.setFrequency(38);

// Header
  IR.mark(AC1_HDR_MARK);
  IR.space(AC1_HDR_SPACE);

// Data
  for (unsigned int i=0; i<sizeof(ZHLT01Template); i++) {
    IR.sendIRbyte(ZHLT01Template[i], AC1_BIT_MARK, AC1_ZERO_SPACE, AC1_ONE_SPACE);
  }

// End mark
  IR.mark(AC1_BIT_MARK);
  IR.space(0);
}
