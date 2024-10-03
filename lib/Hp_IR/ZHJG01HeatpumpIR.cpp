#include <ZHJG01HeatpumpIR.h>

ZHJG01HeatpumpIR::ZHJG01HeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "ZHJG01";
  static const char info[]  PROGMEM = "{\"mdl\":\"ZHJG01\",\"dn\":\"ZHJG01\",\"mT\":18,\"xT\":32,\"fs\":3,\"maint\":[10]}}";

  _model = model;
  _info = info;
}

void ZHJG01HeatpumpIR::send(IRSender& IR,
                            uint8_t powerModeCmd,
                            uint8_t operatingModeCmd,
                            uint8_t fanSpeedCmd,
                            uint8_t temperatureCmd,
                            uint8_t swingVCmd,
                            uint8_t swingHCmd)
{

  uint8_t operatingMode = ZHJG01_MODE_AUTO;
  uint8_t fanSpeed      = ZHJG01_FAN_AUTO;
  uint8_t temperature   = 25;
  uint8_t swingV        = ZHJG01_VDIR_FIXED;

  uint8_t powerMode     = powerModeCmd == 0 ? ZHJG01_POWER_OFF : ZHJG01_POWER_ON;

  switch (operatingModeCmd)
  {
    case MODE_COOL:
      operatingMode  = ZHJG01_MODE_COOL;
      break;
    case MODE_FAN:
      operatingMode  = ZHJG01_MODE_FAN;
      break;
    case MODE_DRY:
      operatingMode  = ZHJG01_MODE_DRY;
      temperatureCmd = 25;
      break;
    case MODE_HEAT:
      operatingMode  = ZHJG01_MODE_HEAT;
      break;
    default:
      operatingMode  = ZHJG01_MODE_AUTO;
      temperatureCmd = 25;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = ZHJG01_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = ZHJG01_FAN1;
      break;
    case FAN_2:
      fanSpeed = ZHJG01_FAN2;
      break;
    case FAN_3:
      fanSpeed = ZHJG01_FAN3;
      break;
    case FAN_4:
    case FAN_5:
      fanSpeed = ZHJG01_FAN_TURBO;
      break;
    case FAN_SILENT:
      fanSpeed = ZHJG01_FAN_ECO;
      break;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = ZHJG01_VDIR_WIND;
      break;
    case VDIR_SWING:
      swingV = ZHJG01_VDIR_SWING;
      break;
  default:
      swingV = ZHJG01_VDIR_FIXED;
  }

  // temperature must be between 17 and 32 degrees
  if (temperatureCmd < 16)
  {
    temperature = 16;
  }
  else if (temperatureCmd > 32)
  {
    temperature = 32;
  } else
  {
    temperature = temperatureCmd;
  }

  sendZHJG01(IR, powerMode, operatingMode, fanSpeed, temperature, swingV);
}

void ZHJG01HeatpumpIR::sendZHJG01(IRSender& IR,
                                  uint8_t powerMode,
                                  uint8_t operatingMode,
                                  uint8_t fanSpeed,
                                  uint8_t temperature,
                                  uint8_t swingV)
{
  uint8_t ZHJG01Template[] = { 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF };
  //    Bytenumbers:             0     1     2     3     4     5

/********************************************************************************
 * Byte[0]: Turbo, Eco, Fan, Vertical Swing
 * TURBO ON:         B0x1xxxxx
 * ECO ON:           B0x0xxxxx
 * TURBO/ECO OFF:    B1xxxxxxx
 * FAN1:             Bx00xxxxx
 * FAN2:             Bx01xxxxx
 * FAN3:             Bx10xxxxx
 * FAN AUTO:         Bx11xxxxx
 * VERTICAL FIXED:   Bxxx01xxx
 * VERTICAL SWING:   Bxxx10xxx
 * VERTICAL WIND:    Bxxx11xxx
 *******************************************************************************/
  ZHJG01Template[1] = fanSpeed | swingV;
  ZHJG01Template[0] = ~ ZHJG01Template[1];

/********************************************************************************
 * Byte[2]: Temp, Power, Mode
 * TEMP:      Bttttxxxx
 * POWER ON:  Bxxxx0xxx
 * POWER OFF: Bxxxx1xxx
 * MODE HEAT: Bxxxxx011
 * MODE VENT: Bxxxxx100
 * MODE DRY:  Bxxxxx101
 * MODE COOL: Bxxxxx110
 * MODE AUTO: Bxxxxx111
 *******************************************************************************/

  uint8_t tempBits = ((temperature - 17) << 4) & 0b11110000;
  ZHJG01Template[3] = tempBits | powerMode | operatingMode;
  ZHJG01Template[2] = ~ ZHJG01Template[3];


  ZHJG01Template[5] = 0b11010101; // Undecoded (timer?)
  ZHJG01Template[4] = ~ ZHJG01Template[5];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(ZHJG01_HDR_MARK);
  IR.space(ZHJG01_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(ZHJG01Template); i++) {
    IR.sendIRbyte(ZHJG01Template[i], ZHJG01_BIT_MARK, ZHJG01_ZERO_SPACE, ZHJG01_ONE_SPACE);
  }

  // End mark
  IR.mark(ZHJG01_BIT_MARK);
  IR.space(0);
}
