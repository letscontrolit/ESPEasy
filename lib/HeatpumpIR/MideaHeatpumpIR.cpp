#include <MideaHeatpumpIR.h>

MideaHeatpumpIR::MideaHeatpumpIR() : HeatpumpIR()
{
  static const char PROGMEM model[]  PROGMEM = "midea"; // The basic and inverter Midea's are the same, i.e. at least Pro Plus 10FP, Pro Plus 13Fp, Pro Plus 9 Inverter and Pro Plus 12 Inverter should work
  static const char PROGMEM info[]   PROGMEM = "{\"mdl\":\"midea\",\"dn\":\"Ultimate Pro Plus\",\"mT\":16,\"xT\":30,\"fs\":4,\"maint\":[10]}";

  _model = model;
  _info = info;
}


void MideaHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode
  uint8_t operatingMode = MIDEA_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = MIDEA_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;

  switch (powerModeCmd)
  {
    case 0:
      // OFF is a special case
      operatingMode = MIDEA_AIRCON1_MODE_OFF;
      sendMidea(IR, operatingMode, fanSpeed, temperature);
      return;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MIDEA_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MIDEA_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MIDEA_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MIDEA_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MIDEA_AIRCON1_MODE_FAN;
      break;
    case MODE_MAINT:
      // Maintenance mode ('FP' on the remote) is a special mode on Midea
      // Also, this is a switch between 'normal' operation and 'maintenance' operation,
      // i.e. if already running on maintenance, the heatpump will go back to normal operation
      operatingMode = MIDEA_AIRCON1_MODE_FP;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MIDEA_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MIDEA_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = MIDEA_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = MIDEA_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 16 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  sendMidea(IR, operatingMode, fanSpeed, temperature);
}

// Send the Midea code
void MideaHeatpumpIR::sendMidea(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature)
{
  uint8_t sendBuffer[3] = { 0x4D, 0x00, 0x00 }; // First uint8_t is always 0x4D

  static const uint8_t temperatures[] PROGMEM = {0, 8, 12, 4, 6, 14, 10, 2, 3, 11, 9, 1, 5, 13 };

  static const uint8_t OffMsg[] PROGMEM = {0x4D, 0xDE, 0x07 };
  static const uint8_t FPMsg[] PROGMEM =  {0xAD, 0xAF, 0xB5 };

  if (operatingMode == MIDEA_AIRCON1_MODE_OFF)
  {
    memcpy_P(sendBuffer, OffMsg, sizeof(sendBuffer));
  }
  else if (operatingMode == MIDEA_AIRCON1_MODE_FP)
  {
    memcpy_P(sendBuffer, FPMsg, sizeof(sendBuffer));
  }
  else
  {
    sendBuffer[1] = ~fanSpeed;

    if ( operatingMode == MIDEA_AIRCON1_MODE_FAN )
    {
      sendBuffer[2] = MIDEA_AIRCON1_MODE_DRY | 0x07;
    }
    else
    {
      sendBuffer[2] = operatingMode | pgm_read_byte(&(temperatures[(temperature-17)]));
    }
  }

  // Send the code
  sendMidearaw(IR, sendBuffer);
}

// Send the Midea raw code
void MideaHeatpumpIR::sendMidearaw(IRSender& IR, uint8_t sendBuffer[])
{
  // 40 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MIDEA_AIRCON1_HDR_MARK);
  IR.space(MIDEA_AIRCON1_HDR_SPACE);

  // Six uint8_ts, every second uint8_t is a bitwise not of the previous uint8_t
  for (int i=0; i<3; i++) {
    IR.sendIRbyte(sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(~sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
  }

  // Pause
  IR.mark(MIDEA_AIRCON1_BIT_MARK);
  IR.space(MIDEA_AIRCON1_MSG_SPACE);

  // Header, two last uint8_ts repeated
  IR.mark(MIDEA_AIRCON1_HDR_MARK);
  IR.space(MIDEA_AIRCON1_HDR_SPACE);

  // Six uint8_ts, every second uint8_t is a bitwise not of the previous uint8_t
  for (int i=0; i<3; i++) {
    IR.sendIRbyte(sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(~sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(MIDEA_AIRCON1_BIT_MARK);
  IR.space(0);
}