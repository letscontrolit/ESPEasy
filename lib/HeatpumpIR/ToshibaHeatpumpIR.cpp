#include <ToshibaHeatpumpIR.h>


ToshibaHeatpumpIR::ToshibaHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "toshiba";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"toshiba\",\"dn\":\"Toshiba\",\"mT\":17,\"xT\":30,\"fs\":5}";

  _model = model;
  _info = info;
}


void ToshibaHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = TOSHIBA_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed      = TOSHIBA_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;

  (void)swingVCmd;
  (void)swingHCmd;


  if (powerModeCmd == POWER_OFF)
  {
    operatingMode = TOSHIBA_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = TOSHIBA_AIRCON1_MODE_AUTO;
        break;
      case MODE_COOL:
        operatingMode = TOSHIBA_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = TOSHIBA_AIRCON1_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = TOSHIBA_AIRCON1_MODE_COOL;
        temperatureCmd = 30;
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = TOSHIBA_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = TOSHIBA_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = TOSHIBA_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = TOSHIBA_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = TOSHIBA_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = TOSHIBA_AIRCON1_FAN5;
      break;
  }

  if (temperatureCmd > 16 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  sendToshiba(IR, operatingMode, fanSpeed, temperature, 0, 0);
}


void ToshibaHeatpumpIR::sendToshiba(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t ToshibaTemplate[] = { 0x4F, 0xB0, 0xC0, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00 };
  //                             0     1     2     3     4     5     6     7     8

  uint8_t checksum = 0x00;
  (void)swingV;
  (void)swingH;

  // Set the operatingmode on the template message
  ToshibaTemplate[6] |= operatingMode | fanSpeed;

  // Set the temperature on the template message
  ToshibaTemplate[5] |= (IR.bitReverse(temperature - 17)) >> 4;

  // Calculate the checksum
  for (uint8_t i = 0; i < 8; i++) {
    checksum ^= ToshibaTemplate[i];
  }

  ToshibaTemplate[8] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(TOSHIBA_AIRCON1_HDR_MARK);
  IR.space(TOSHIBA_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(ToshibaTemplate); i++) {
    IR.sendIRbyte(ToshibaTemplate[i], TOSHIBA_AIRCON1_BIT_MARK, TOSHIBA_AIRCON1_ZERO_SPACE, TOSHIBA_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(TOSHIBA_AIRCON1_BIT_MARK);
  IR.space(0);
}