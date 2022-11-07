#include <AUXHeatpumpIR.h>

AUXHeatpumpIR::AUXHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "AUX";
  static const char info[]  PROGMEM = "{\"mdl\":\"aux\",\"dn\":\"AUX\",\"mT\":16,\"xT\":30,\"fs\":5}";

  _model = model;
  _info = info;
}


void AUXHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode = AUX_AIRCON1_MODE_ON;
  uint8_t operatingMode = AUX_AIRCON1_MODE_AUTO;
  uint8_t fanSpeed = AUX_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;
  uint8_t swingV = AUX_AIRCON1_VDIR_MANUAL;
  uint8_t swingH = AUX_AIRCON1_HDIR_MANUAL;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = AUX_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = AUX_AIRCON1_MODE_AUTO;
        break;
      case MODE_HEAT:
        operatingMode = AUX_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = AUX_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = AUX_AIRCON1_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = AUX_AIRCON1_MODE_FAN;
       break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = AUX_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = AUX_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = AUX_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = AUX_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  if (swingVCmd == VDIR_SWING)
  {
    swingV = AUX_AIRCON1_VDIR_SWING;
  }

    if (swingHCmd == HDIR_SWING)
  {
    swingH = AUX_AIRCON1_HDIR_SWING;
  }

  sendAUX(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}


void AUXHeatpumpIR::sendAUX(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  // ON, HEAT, AUTO FAN, +24 degrees
  uint8_t AUXTemplate[] = { 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00 };
  //                           0     1     2     3     4     5     6     7     8     9    10    11    12

  uint8_t checksum = 0x00;

  // Set the power mode on the template message
  AUXTemplate[9] |= powerMode;

  // Set the operatingmode on the template message
  AUXTemplate[6] |= operatingMode;

  // Set the temperature on the template message
  AUXTemplate[1] |= (temperature - 8) << 3;

  // Set the fan speed on the template message
  AUXTemplate[4] |= fanSpeed;

  // Set the vertical air direction on the template message
  AUXTemplate[1] |= swingV;

  // Set the horizontal air direction on the template message
  AUXTemplate[2] |= swingH;

  // Calculate the checksum
  for (uint8_t i = 0; i < 12; i++) {
    checksum += AUXTemplate[i];
  }

  AUXTemplate[12] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(AUX_AIRCON1_HDR_MARK);
  IR.space(AUX_AIRCON1_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(AUXTemplate); i++) {
    IR.sendIRbyte(AUXTemplate[i], AUX_AIRCON1_BIT_MARK, AUX_AIRCON1_ZERO_SPACE, AUX_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(AUX_AIRCON1_BIT_MARK);
  IR.space(0);
}
