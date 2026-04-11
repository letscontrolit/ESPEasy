#include <PhilcoPHS32HeatpumpIR.h>

PhilcoPHS32HeatpumpIR::PhilcoPHS32HeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "philco_phs32";
  static const char info[]  PROGMEM = "{\"mdl\":\"philco_phs32\",\"dn\":\"Philco PHS32\"}";

  _model = model;
  _info = info;
}


void PhilcoPHS32HeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd , uint8_t fanSpeedCmd , uint8_t temperatureCmd , uint8_t swingVCmd , uint8_t swingHCmd )
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode
  uint8_t powerMode = PHILCO_AIRCON1_POWER_ON;
  uint8_t operatingMode = PHILCO_AIRCON1_MODE_COOL;
  uint8_t fanSpeed = PHILCO_AIRCON1_FAN_AUTO;
  uint8_t temperature = 24;
  uint8_t swingV=0;
  uint8_t swingH=0;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = PHILCO_AIRCON1_POWER_OFF;
    temperature = PHILCO_AIRCON1_OFF_TEMP;
    operatingMode = PHILCO_AIRCON1_MODE_COOL;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_HEAT:
        operatingMode = PHILCO_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = PHILCO_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = PHILCO_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        temperatureCmd = PHILCO_AIRCON1_DRY_TEMP; // Fixed temperature DRY mode
        break;
      case MODE_FAN:
        operatingMode = PHILCO_AIRCON1_MODE_FAN;
        temperatureCmd = PHILCO_AIRCON1_FAN_TEMP; // Fixed temperature FAN mode
        if ( fanSpeedCmd == FAN_AUTO ) {
          fanSpeedCmd = FAN_2; // Fan speed cannot be 'AUTO' in FAN mode
        };
        break;
    }
    if ( temperatureCmd > 17 && temperatureCmd < 31)
    {
      temperature = temperatureCmd;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = PHILCO_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = PHILCO_AIRCON1_FAN_LOW;
      break;
    case FAN_2:
      fanSpeed = PHILCO_AIRCON1_FAN_LOW;
      break;
    case FAN_3:
      fanSpeed = PHILCO_AIRCON1_FAN_MED;
      break;
    case FAN_4:
      fanSpeed = PHILCO_AIRCON1_FAN_HIGH;
      break;
    case FAN_5:
      fanSpeed = PHILCO_AIRCON1_FAN_HIGH;
      break;
  }

  sendPhilco(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

// Send the Philco code
void PhilcoPHS32HeatpumpIR::sendPhilco(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV ,uint8_t swingH)
{
  (void)swingV;
  (void)swingH;

  uint8_t PhilcoTemplate[] = { 0x83, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00 };

  uint8_t  i;

  // Set Power state
  PhilcoTemplate[18] = powerMode;
  // Set the Fan speed, Temperature and Operating Mode
  PhilcoTemplate[2] += fanSpeed;
  PhilcoTemplate[3] = (temperature - 18) << 4;
  PhilcoTemplate[3] += operatingMode;

  // Calculate checksums
  for (int i = 2; i < 13; i++) {
    PhilcoTemplate[13] ^= PhilcoTemplate[i];
  };
  for (int i = 14; i < 20; i++) {
    PhilcoTemplate[20] ^= PhilcoTemplate[i];
  };

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header
  IR.mark(PHILCO_AIRCON1_HDR_MARK);
  IR.space(PHILCO_AIRCON1_HDR_SPACE);

  // Send 6 bytes of Data
  for (unsigned int i=0; i<6; i++) {
    IR.sendIRbyte(PhilcoTemplate[i], PHILCO_AIRCON1_BIT_MARK, PHILCO_AIRCON1_ZERO_SPACE, PHILCO_AIRCON1_ONE_SPACE);
  }

  // Send Message space
  IR.mark(PHILCO_AIRCON1_BIT_MARK);
  IR.space(PHILCO_AIRCON1_MSG_SPACE);

  // Send 8 bytes of Data
  for (unsigned int i=6; i<14; i++) {
    IR.sendIRbyte(PhilcoTemplate[i], PHILCO_AIRCON1_BIT_MARK, PHILCO_AIRCON1_ZERO_SPACE, PHILCO_AIRCON1_ONE_SPACE);
  }

  // Send Message space
  IR.mark(PHILCO_AIRCON1_BIT_MARK);
  IR.space(PHILCO_AIRCON1_MSG_SPACE);

  // Send 7 bytes of Data
  for (unsigned int i=14; i<21; i++) {
    IR.sendIRbyte(PhilcoTemplate[i], PHILCO_AIRCON1_BIT_MARK, PHILCO_AIRCON1_ZERO_SPACE, PHILCO_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(PHILCO_AIRCON1_BIT_MARK);
  IR.space(0);
}