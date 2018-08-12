#include <FuegoHeatpumpIR.h>


FuegoHeatpumpIR::FuegoHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "fuego";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"fuego\",\"dn\":\"Fuego\",\"mT\":18,\"xT\":31,\"fs\":3}";

  _model = model;
  _info = info;
}


void FuegoHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = FUEGO_AIRCON1_MODE_ON;
  uint8_t operatingMode = FUEGO_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed      = FUEGO_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = FUEGO_AIRCON1_VS_SWING;

  (void)swingHCmd;


  if (powerModeCmd == POWER_OFF)
  {
    powerMode = FUEGO_AIRCON1_MODE_OFF;
    swingVCmd = VDIR_MUP;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = FUEGO_AIRCON1_MODE_AUTO;
      break;
    case MODE_COOL:
      operatingMode = FUEGO_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = FUEGO_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = FUEGO_AIRCON1_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = FUEGO_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = FUEGO_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = FUEGO_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = FUEGO_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 17 && temperatureCmd < 32)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = FUEGO_AIRCON1_VS_AUTO;
      break;
    case VDIR_SWING:
      swingV = FUEGO_AIRCON1_VS_SWING;
      break;
    case VDIR_UP:
      swingV = FUEGO_AIRCON1_VS_UP;
      break;
    case VDIR_MUP:
      swingV = FUEGO_AIRCON1_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = FUEGO_AIRCON1_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = FUEGO_AIRCON1_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = FUEGO_AIRCON1_VS_DOWN;
      break;
  }

  sendFuego(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, 0);
}


void FuegoHeatpumpIR::sendFuego(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t FuegoTemplate[] = { 0x23, 0xCB, 0x26, 0x01, 0x80, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //                             0     1     2     3     4     5     6     7     8     9    10    11    12    13

  uint8_t checksum = 0x00;
  (void)swingH;

  // Set the operatingmode on the template message
  FuegoTemplate[5] |= powerMode;
  FuegoTemplate[6] |= operatingMode;

  // Set the temperature on the template message
  FuegoTemplate[7] |= 31 - temperature;

  // Set the fan speed and vertical air direction on the template message
  FuegoTemplate[8] |= fanSpeed | swingV;

  // Calculate the checksum
  for (unsigned int i=0; i < (sizeof(FuegoTemplate)-1); i++) {
    checksum += FuegoTemplate[i];
  }

  FuegoTemplate[13] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(FUEGO_AIRCON1_HDR_MARK);
  IR.space(FUEGO_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(FuegoTemplate); i++) {
    IR.sendIRbyte(FuegoTemplate[i], FUEGO_AIRCON1_BIT_MARK, FUEGO_AIRCON1_ZERO_SPACE, FUEGO_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(FUEGO_AIRCON1_BIT_MARK);
  IR.space(0);
}