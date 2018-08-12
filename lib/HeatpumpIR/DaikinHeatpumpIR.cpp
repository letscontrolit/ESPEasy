#include <DaikinHeatpumpIR.h>

DaikinHeatpumpIR::DaikinHeatpumpIR() : HeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "daikin";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"daikin\",\"dn\":\"Daikin\",\"mT\":18,\"xT\":30,\"fs\":6,\"maint\":[10,11,12,13,14,15,16,17]}}}";

  _model = model;
  _info = info;
}


// Daikin numeric values to command bytes
void DaikinHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = DAIKIN_AIRCON_MODE_OFF | DAIKIN_AIRCON_MODE_AUTO;
  uint8_t fanSpeed      = DAIKIN_AIRCON_FAN_AUTO;
  uint8_t temperature   = 23;

  switch (powerModeCmd)
  {
    case POWER_ON:
      operatingMode |= DAIKIN_AIRCON_MODE_ON;
      break;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode |= DAIKIN_AIRCON_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode |= DAIKIN_AIRCON_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode |= DAIKIN_AIRCON_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode |= DAIKIN_AIRCON_MODE_DRY;
      temperatureCmd = 0x24;
      break;
    case MODE_FAN:
      operatingMode |= DAIKIN_AIRCON_MODE_FAN;
      temperatureCmd = 0xC0;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = DAIKIN_AIRCON_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = DAIKIN_AIRCON_FAN1;
      break;
    case FAN_2:
      fanSpeed = DAIKIN_AIRCON_FAN2;
      break;
    case FAN_3:
      fanSpeed = DAIKIN_AIRCON_FAN3;
      break;
    case FAN_4:
      fanSpeed = DAIKIN_AIRCON_FAN4;
      break;
    case FAN_5:
      fanSpeed = DAIKIN_AIRCON_FAN5;
      break;
  }

  if ((operatingModeCmd == MODE_HEAT && temperatureCmd >= 10 && temperatureCmd <= 30) ||
      (temperatureCmd >= 18 && temperatureCmd <= 30))
  {
    temperature = temperatureCmd << 1;
  }

  sendDaikin(IR, operatingMode, fanSpeed, temperature, swingVCmd, swingHCmd);
}


// Send the Daikin code
void DaikinHeatpumpIR::sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  (void)swingV;
  (void)swingH;

  uint8_t daikinTemplate[35] = {
    0x11, 0xDA, 0x27, 0x00, 0xC5, 0x00, 0x00, 0xD7, // First header
   //  0     1     2     3     4     5     6     7
    0x11, 0xDA, 0x27, 0x00, 0x42, 0x49, 0x05, 0xA2, // Second header, this seems to have the wall clock time (bytes 12 & 13 are changing)
   //  8     9    10    11    12    13    14    15
    0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00 };
   // 16    17    18    19    20    21    22    23    24    25   26     27    28    29    30    31    32    33    34

  daikinTemplate[21] = operatingMode;
  daikinTemplate[22] = temperature;
  daikinTemplate[24] = fanSpeed;

  // Checksum calculation
  // * Checksums at bytes 7 and 15 are calculated the same way
  uint8_t checksum = 0x00;

  for (int i=16; i<34; i++) {
    checksum += daikinTemplate[i];
  }

  daikinTemplate[34] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(DAIKIN_AIRCON_HDR_MARK);
  IR.space(DAIKIN_AIRCON_HDR_SPACE);

  // First header
  for (int i=0; i<8; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(DAIKIN_AIRCON_MSG_SPACE);
  IR.mark(DAIKIN_AIRCON_HDR_MARK);
  IR.space(DAIKIN_AIRCON_HDR_SPACE);

  // Second header - this probably has the wall clock time
  for (int i=8; i<16; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(DAIKIN_AIRCON_MSG_SPACE);
  IR.mark(DAIKIN_AIRCON_HDR_MARK);
  IR.space(DAIKIN_AIRCON_HDR_SPACE);

  // Last 19 bytes - the actual payload
  for (int i=16; i<35; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(0);
}