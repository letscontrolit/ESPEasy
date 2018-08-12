#include <DaikinHeatpumpARC417IR.h>

DaikinHeatpumpARC417IR::DaikinHeatpumpARC417IR() : HeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "daikin_arc417";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"daikin_arc417\",\"dn\":\"Daikin ARC417\",\"mT\":18,\"xT\":30,\"fs\":6,\"maint\":[10,11,12,13,14,15,16,17]}}}";

  _model = model;
  _info = info;
}


// Daikin numeric values to command bytes
void DaikinHeatpumpARC417IR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = DAIKIN_AIRCON2_MODE_OFF | DAIKIN_AIRCON2_MODE_AUTO;
  uint8_t fanSpeed      = DAIKIN_AIRCON2_FAN_AUTO;
  uint8_t temperature   = 0x10; // 18 deg

  if ((operatingModeCmd == MODE_HEAT && temperatureCmd >= 14 && temperatureCmd <= 28) ||
      (operatingModeCmd == MODE_COOL && temperatureCmd >= 18 && temperatureCmd <= 32) ||
      (temperatureCmd >= 18 && temperatureCmd <= 28))
  {
    temperature = (temperatureCmd << 1) - 20;
  }

  switch (powerModeCmd)
  {
    case POWER_ON:
      operatingMode |= DAIKIN_AIRCON2_MODE_ON;
      break;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode |= DAIKIN_AIRCON2_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode |= DAIKIN_AIRCON2_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode |= DAIKIN_AIRCON2_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode |= DAIKIN_AIRCON2_MODE_DRY;
      temperature = 0x80;
      break;
    case MODE_FAN:
      operatingMode |= DAIKIN_AIRCON2_MODE_FAN;
      temperatureCmd = 0xC0;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = DAIKIN_AIRCON2_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = DAIKIN_AIRCON2_FAN1;
      break;
    case FAN_2:
      fanSpeed = DAIKIN_AIRCON2_FAN2;
      break;
    case FAN_3:
      fanSpeed = DAIKIN_AIRCON2_FAN3;
      break;
    case FAN_4:
      fanSpeed = DAIKIN_AIRCON2_FAN4;
      break;
    case FAN_5:
      fanSpeed = DAIKIN_AIRCON2_FAN5;
      break;
  }

  sendDaikin(IR, operatingMode, fanSpeed, temperature, swingVCmd, swingHCmd);
}


// Send the Daikin code
void DaikinHeatpumpARC417IR::sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  (void)swingV;
  (void)swingH;

  uint8_t daikinTemplate[20] = {
    0x11, 0xDA, 0x27, 0xF0, 0x0D, 0x00, 0x0F, // First header
   //  0     1     2     3     4     5     6
    0x11, 0xDA, 0x27, 0x00, 0xD3, 0x11, 0x00, 0x00, 0x00, 0x1E, 0x0A, 0x08, 0x26 };
   //  7     8     9    10    11    12    13    14    15    16    17    18    19

  daikinTemplate[12] = operatingMode;
  daikinTemplate[16] = temperature;
  daikinTemplate[17] = fanSpeed;
  
  // Checksum calculation
  // * Checksum at byte 6 is calculated the same way
  uint8_t checksum = 0x00;

  for (int i=7; i<19; i++) {
    checksum += daikinTemplate[i];
  }

  daikinTemplate[19] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(DAIKIN_AIRCON2_HDR_MARK);
  IR.space(DAIKIN_AIRCON2_HDR_SPACE);

  // First header
  for (int i=0; i<7; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON2_BIT_MARK, DAIKIN_AIRCON2_ZERO_SPACE, DAIKIN_AIRCON2_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(DAIKIN_AIRCON2_BIT_MARK);
  IR.space(DAIKIN_AIRCON2_MSG_SPACE);
  IR.mark(DAIKIN_AIRCON2_HDR_MARK);
  IR.space(DAIKIN_AIRCON2_HDR_SPACE);

  // 
  for (int i=7; i<20; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON2_BIT_MARK, DAIKIN_AIRCON2_ZERO_SPACE, DAIKIN_AIRCON2_ONE_SPACE);
  }

  IR.mark(DAIKIN_AIRCON2_BIT_MARK);
  IR.space(0);
}