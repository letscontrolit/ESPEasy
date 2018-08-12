#include <BalluHeatpumpIR.h>

BalluHeatpumpIR::BalluHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "ballu";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"ballu\",\"dn\":\"Ballu\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}


void BalluHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd)
{
  // Sensible defaults for the heat pump mode
  
  uint8_t operatingMode = BALLU_AIRCON_MODE_COOL;
  uint8_t fanSpeed      = BALLU_AIRCON_FAN_AUTO;
  uint8_t temperature   = 21;
  uint8_t powerMode     = 00;
  
  
  if (powerModeCmd == POWER_OFF)
  {
    powerMode = BALLU_AIRCON_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_COOL:
        operatingMode = BALLU_AIRCON_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = BALLU_AIRCON_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = BALLU_AIRCON_MODE_FAN;
        //temperatureCmd = 30;
        break;
      case MODE_HEAT:
        operatingMode = BALLU_AIRCON_MODE_HEAT;
        break;
    }
  }
  
  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = BALLU_AIRCON_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = BALLU_AIRCON_FAN1;
      break;
    case FAN_2:
      fanSpeed = BALLU_AIRCON_FAN2;
      break;
    case FAN_3:
      fanSpeed = BALLU_AIRCON_FAN3;
      break;
  }
  
  if (temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }
  
  sendBallu(IR, powerMode, operatingMode, fanSpeed, temperature);
}


void BalluHeatpumpIR::sendBallu(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature)
{
  uint8_t BalluTemplate[] = { 0x83, 0x06, 0x04, 0x42, 0x00, 0x00 };
  //                             0     1     2     3     4     5

  BalluTemplate[2] = fanSpeed;

  BalluTemplate[3] =(temperature - 16) << 4;
  BalluTemplate[3] |= operatingMode;

  if (powerMode==BALLU_AIRCON_MODE_OFF) //83,06,04,72,00,00
  {
    BalluTemplate[2] = powerMode;
    //BalluTemplate[3] = 0x72;
  }

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(BALLU_AIRCON_HDR_MARK);
  IR.space(BALLU_AIRCON_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(BalluTemplate); i++) {
    IR.sendIRbyte(BalluTemplate[i], BALLU_AIRCON_BIT_MARK, BALLU_AIRCON_ZERO_SPACE, BALLU_AIRCON_ONE_SPACE);
  }

  // End mark
  IR.mark(BALLU_AIRCON_BIT_MARK);
  IR.space(0);
}
