#include <HitachiHeatpumpIR.h>

HitachiHeatpumpIR::HitachiHeatpumpIR() : HeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "hitachi";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"hitachi\",\"dn\":\"Hitachi\",\"mT\":16,\"xT\":32,\"fs\":4}";

  _model = model;
  _info = info;
}

void HitachiHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVcmd, uint8_t swingHcmd)
{
  // Sensible defaults for the heat pump mode
  uint8_t operatingMode	= HITACHI_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = HITACHI_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;
  uint8_t powerMode;
  uint8_t swingV = HITACHI_AIRCON1_VDIR_AUTO;
  uint8_t swingH = HITACHI_AIRCON1_HDIR_AUTO;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = HITACHI_AIRCON1_POWER_OFF;
  }
  else
  {
    powerMode = HITACHI_AIRCON1_POWER_ON;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = HITACHI_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = HITACHI_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = HITACHI_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = HITACHI_AIRCON1_MODE_DRY;
      fanSpeedCmd = FAN_2; //Only speed 1 & 2 in dry mode
      break;
    case MODE_FAN:
      operatingMode = HITACHI_AIRCON1_MODE_FAN;
      temperatureCmd = 64; //Temperature = 64 in fan mode
      if (fanSpeedCmd == FAN_AUTO)
      {
        fanSpeedCmd = FAN_2; //No auto fan in fan mode
      }
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = HITACHI_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = HITACHI_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = HITACHI_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = HITACHI_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = HITACHI_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = HITACHI_AIRCON1_FAN4;
      break;
  }

  if ((temperatureCmd > 15 && temperatureCmd < 33) || temperatureCmd == 64)
  {
    temperature = temperatureCmd;
  }

  switch (swingV)
  {
    case HDIR_AUTO:
      swingV = HITACHI_AIRCON1_VDIR_AUTO;
      break;
    case HDIR_SWING:
      swingV = HITACHI_AIRCON1_VDIR_SWING;
      break;
  }

  switch (swingH)
  {
    case HDIR_AUTO:
      swingH = HITACHI_AIRCON1_HDIR_AUTO;
      break;
    case HDIR_SWING:
      swingH = HITACHI_AIRCON1_HDIR_SWING;
      break;
  }

  sendHitachi(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

// Send the Hitachi code
void HitachiHeatpumpIR::sendHitachi(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{

  uint8_t hitachiTemplate[28] = {
    0x01, 0x10, 0x30, 0x40, 0xBF, 0x01, 0xFE, 0x11, 0x12, 0x08, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00 };
    // 0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27

  if(temperature == 16){
    hitachiTemplate[9] = 0x09;
  }
  hitachiTemplate[10] = operatingMode;
  hitachiTemplate[11] = temperature;
  hitachiTemplate[11] = (hitachiTemplate[11] << 1);
  hitachiTemplate[13] = fanSpeed;
  hitachiTemplate[14] |= swingV;
  hitachiTemplate[15] |= swingH;
  hitachiTemplate[17] = powerMode;
  //hitachiTemplate[25] = ecoMode;

  //Checksum calculation
  int checksum = 1086;
  for (byte i = 0; i < 27; i++) {
    checksum -= hitachiTemplate[i];
  }
  hitachiTemplate[27] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(HITACHI_AIRCON1_HDR_MARK);
  IR.space(HITACHI_AIRCON1_HDR_SPACE);

  // Data
  for (int i=0; i<28; i++) {
    IR.sendIRbyte(hitachiTemplate[i], HITACHI_AIRCON1_BIT_MARK, HITACHI_AIRCON1_ZERO_SPACE, HITACHI_AIRCON1_ONE_SPACE);
  }
  IR.mark(HITACHI_AIRCON1_BIT_MARK);
  IR.space(0);
}
