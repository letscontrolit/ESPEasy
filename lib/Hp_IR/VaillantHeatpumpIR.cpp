#include "VaillantHeatpumpIR.h"


VaillantHeatpumpIR::VaillantHeatpumpIR()
{
  static const char model[] PROGMEM = "vaillantvai8";
  static const char info[]  PROGMEM = "{\"mdl\":\"vaillantvai8\",\"dn\":\"Vaillant VAI8\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

void VaillantHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, false, true);
}

void VaillantHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, bool turboModeCmd, bool lightCmd)
{

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = VAILLANT_AIRCON1_POWER_ON;
  uint8_t operatingMode = VAILLANT_AIRCON1_MODE_COOL;
  uint8_t fanSpeed = VAILLANT_AIRCON1_FAN1;
  uint8_t temperature = 25;
  uint8_t swingV = VAILLANT_VDIR_UP;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = VAILLANT_AIRCON1_POWER_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = VAILLANT_AIRCON1_MODE_AUTO;
        temperatureCmd = 25;
        break;
      case MODE_HEAT:
        operatingMode = VAILLANT_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = VAILLANT_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = VAILLANT_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_1;
        break;
      case MODE_FAN:
        operatingMode = VAILLANT_AIRCON1_MODE_FAN;
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = VAILLANT_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = VAILLANT_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = VAILLANT_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = VAILLANT_AIRCON1_FAN3;
      break;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = VAILLANT_VDIR_AUTO;
      break;
    case VDIR_SWING:
      swingV = VAILLANT_VDIR_SWING;
      break;
    case VDIR_UP:
      swingV = VAILLANT_VDIR_UP;
      break;
    case VDIR_MUP:
      swingV = VAILLANT_VDIR_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = VAILLANT_VDIR_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = VAILLANT_VDIR_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = VAILLANT_VDIR_DOWN;
      break;
  }

  if (temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd - 16;
  }

  sendVaillant(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, turboModeCmd, lightCmd);
}

// Send the Vaillant code
void VaillantHeatpumpIR::sendVaillant(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, bool turboMode, bool light)
{

  uint8_t vaillantTemplate[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //                             0     1     2     3     4     5     6     7

  uint8_t i;

  // Set the Fan speed, operating mode and power state
  vaillantTemplate[0] = fanSpeed | operatingMode | powerMode;
  // Set the temperature
  vaillantTemplate[1] = temperature;


  if (powerMode == VAILLANT_AIRCON1_POWER_ON)
  {
    vaillantTemplate[2] = 0x40; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
  }
  
  vaillantTemplate[3] = 0x50;
  vaillantTemplate[4] = swingV;
  vaillantTemplate[5] = 0x02;
  vaillantTemplate[6] = 0x00;

  if (turboMode)
  {
    vaillantTemplate[2] |= (1 << 4); // Set bit 4 (TURBO)
  }
  if (light)
  {
    vaillantTemplate[2] |= (1 << 5); // Set bit 5 (light)
  }
  if (swingV == VAILLANT_VDIR_SWING)
  {
    vaillantTemplate[0] |= (1 << 6); // Enable swing by setting bit 6
  }

  // Calculate the checksum
  vaillantTemplate[7] = (((
   (vaillantTemplate[0] & 0x0F) +
   (vaillantTemplate[1] & 0x0F) +
   (vaillantTemplate[2] & 0x0F) +
   (vaillantTemplate[3] & 0x0F) +
   ((vaillantTemplate[5] & 0xF0) >> 4) +
   ((vaillantTemplate[6] & 0xF0) >> 4) +
   ((vaillantTemplate[7] & 0xF0) >> 4) +
    0x0A) & 0x0F) << 4) | (vaillantTemplate[7] & 0x0F);

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(VAILLANT_AIRCON1_HDR_MARK);
  IR.space(VAILLANT_AIRCON1_HDR_SPACE);

  // Payload part #1
  for (i=0; i<4; i++) {
    IR.sendIRbyte(vaillantTemplate[i], VAILLANT_AIRCON1_BIT_MARK, VAILLANT_AIRCON1_ZERO_SPACE, VAILLANT_AIRCON1_ONE_SPACE);
  }
  // Only three first bits of byte 4 are sent, this is always '010'
  IR.mark(VAILLANT_AIRCON1_BIT_MARK);
  IR.space(VAILLANT_AIRCON1_ZERO_SPACE);
  IR.mark(VAILLANT_AIRCON1_BIT_MARK);
  IR.space(VAILLANT_AIRCON1_ONE_SPACE);
  IR.mark(VAILLANT_AIRCON1_BIT_MARK);
  IR.space(VAILLANT_AIRCON1_ZERO_SPACE);

  // Message space
  IR.mark(VAILLANT_AIRCON1_BIT_MARK);
  IR.space(VAILLANT_AIRCON1_MSG_SPACE);

  // Payload part #2
  for (i=4; i<8; i++) {
    IR.sendIRbyte(vaillantTemplate[i], VAILLANT_AIRCON1_BIT_MARK, VAILLANT_AIRCON1_ZERO_SPACE, VAILLANT_AIRCON1_ONE_SPACE);
	}

  // End mark
  IR.mark(VAILLANT_AIRCON1_BIT_MARK);
  IR.space(0);
}
