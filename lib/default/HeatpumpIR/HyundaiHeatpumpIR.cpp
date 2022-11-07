#include <HyundaiHeatpumpIR.h>

HyundaiHeatpumpIR::HyundaiHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "hyundai";
  static const char info[]  PROGMEM = "{\"mdl\":\"hyundai\",\"dn\":\"Hyundai\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}


void HyundaiHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode
  uint8_t powerMode     = HYUNDAI_AIRCON1_MODE_ON;
  uint8_t operatingMode = HYUNDAI_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed      = HYUNDAI_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = HYUNDAI_AIRCON1_VS_AUTO;
  uint8_t swingH        = 0;

  (void)swingHCmd;

  if (powerModeCmd == 0)
  {
    powerMode = HYUNDAI_AIRCON1_MODE_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = HYUNDAI_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = HYUNDAI_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = HYUNDAI_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = HYUNDAI_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = HYUNDAI_AIRCON1_MODE_FAN;
      temperatureCmd = 24;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = HYUNDAI_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = HYUNDAI_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = HYUNDAI_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = HYUNDAI_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = HYUNDAI_AIRCON1_VS_SWING;
      break;
  }

  sendHyundai(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

void HyundaiHeatpumpIR::sendHyundai(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t HyundaiTemplate[] = { 0x00, 0x00, 0x00, 0x50 };
  //                               0     1     2     3

  (void)swingH;

  // Almost everything goes in the first byte...
  HyundaiTemplate[0] = swingV | fanSpeed | powerMode | operatingMode;

  // Temperature
  HyundaiTemplate[1] = temperature - 16;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(HYUNDAI_AIRCON1_HDR_MARK);
  IR.space(HYUNDAI_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(HyundaiTemplate); i++) {
    IR.sendIRbyte(HyundaiTemplate[i], HYUNDAI_AIRCON1_BIT_MARK, HYUNDAI_AIRCON1_ZERO_SPACE, HYUNDAI_AIRCON1_ONE_SPACE);
  }

  // Hyundai protocol has 35 bits, i.e. 4 bytes + 3 bits
  // The last bits are always '010'
  IR.mark(HYUNDAI_AIRCON1_BIT_MARK);
  IR.space(HYUNDAI_AIRCON1_ZERO_SPACE);
  IR.mark(HYUNDAI_AIRCON1_BIT_MARK);
  IR.space(HYUNDAI_AIRCON1_ONE_SPACE);
  IR.mark(HYUNDAI_AIRCON1_BIT_MARK);
  IR.space(HYUNDAI_AIRCON1_ZERO_SPACE);

  // End mark
  IR.mark(HYUNDAI_AIRCON1_BIT_MARK);
  IR.space(0);
}