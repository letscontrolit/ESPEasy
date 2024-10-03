#include <AIRWAYHeatpumpIR.h>


AIRWAYHeatpumpIR::AIRWAYHeatpumpIR()
{
  static const char model[] PROGMEM = "AIRWAY";
  static const char info[]  PROGMEM = "{\"mdl\":\"AIRWAY\",\"dn\":\"AIRWAY\",\"mT\":18,\"xT\":31,\"fs\":3}";

  _model = model;
  _info = info;
}


void AIRWAYHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = AIRWAY_AIRCON1_MODE_ON;
  uint8_t operatingMode = AIRWAY_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed      = AIRWAY_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = AIRWAY_AIRCON1_VS_SWING;

  (void)swingHCmd;


  if (powerModeCmd == POWER_OFF)
  {
    powerMode = AIRWAY_AIRCON1_MODE_OFF;
    swingVCmd = VDIR_MUP;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = AIRWAY_AIRCON1_MODE_AUTO;
      break;
    case MODE_COOL:
      operatingMode = AIRWAY_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = AIRWAY_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = AIRWAY_AIRCON1_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = AIRWAY_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = AIRWAY_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = AIRWAY_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = AIRWAY_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 17 && temperatureCmd < 32)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = AIRWAY_AIRCON1_VS_AUTO;
      break;
    case VDIR_SWING:
      swingV = AIRWAY_AIRCON1_VS_SWING;
      break;
    case VDIR_UP:
      swingV = AIRWAY_AIRCON1_VS_UP;
      break;
    case VDIR_MUP:
      swingV = AIRWAY_AIRCON1_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = AIRWAY_AIRCON1_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = AIRWAY_AIRCON1_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = AIRWAY_AIRCON1_VS_DOWN;
      break;
  }

  sendAIRWAY(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, 0);
}


void AIRWAYHeatpumpIR::sendAIRWAY(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t AIRWAYTemplate[] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //                             0     1     2     3     4     5     6     7     8     9    10    11    12    13

  uint8_t checksum = 0x00;
  (void)swingH;

  // Set the operatingmode on the template message
  AIRWAYTemplate[5] |= powerMode;
  AIRWAYTemplate[6] |= operatingMode;

  // Set the temperature on the template message
  AIRWAYTemplate[7] |= 31 - temperature;

  // Set the fan speed and vertical air direction on the template message
  AIRWAYTemplate[8] |= fanSpeed | swingV;
  // Calculate the checksum
  for (unsigned int i=0; i < (sizeof(AIRWAYTemplate)-1); i++) {
    checksum += AIRWAYTemplate[i];
  }

  AIRWAYTemplate[13] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(AIRWAY_AIRCON1_HDR_MARK);
  IR.space(AIRWAY_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(AIRWAYTemplate); i++) {
    IR.sendIRbyte(AIRWAYTemplate[i], AIRWAY_AIRCON1_BIT_MARK, AIRWAY_AIRCON1_ZERO_SPACE, AIRWAY_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(AIRWAY_AIRCON1_BIT_MARK);
  IR.space(0);
}