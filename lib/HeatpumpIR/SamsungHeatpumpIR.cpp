#include <SamsungHeatpumpIR.h>

// These are protected methods, i.e. generic Samsung instances cannot be created directly
SamsungHeatpumpIR::SamsungHeatpumpIR() : HeatpumpIR()
{
}

void SamsungHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
}

// Samsung models

SamsungAQVHeatpumpIR::SamsungAQVHeatpumpIR() : SamsungHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "samsung_aqv";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"samsung_aqv\",\"dn\":\"Samsung AQV\",\"mT\":16,\"xT\":27,\"fs\":4}";

  _model = model;
  _info = info;
}

SamsungFJMHeatpumpIR::SamsungFJMHeatpumpIR() : SamsungHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "samsung_fjm";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"samsung_fjm\",\"dn\":\"Samsung FJM\",\"mT\":16,\"xT\":27,\"fs\":4}";

  _model = model;
  _info = info;
}

// Samsung AQV12PSBN / AQV09ASA heatpump control (remote control P/N zzz)

void SamsungAQVHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = SAMSUNG_AIRCON1_MODE_ON;
  uint8_t operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;
  uint8_t swingV = SAMSUNG_AIRCON1_VS_AUTO;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = SAMSUNG_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = SAMSUNG_AIRCON1_MODE_AUTO;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in AUTO mode
        break;
      case MODE_HEAT:
        operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = SAMSUNG_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = SAMSUNG_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = SAMSUNG_AIRCON1_MODE_FAN;
        if ( fanSpeedCmd == FAN_AUTO ) {
          fanSpeedCmd = FAN_1; // Fan speed cannot be 'AUTO' in FAN mode
        }
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = SAMSUNG_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = SAMSUNG_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = SAMSUNG_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 28)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = SAMSUNG_AIRCON1_VS_SWING;
      break;
  }

  sendSamsung(IR, powerMode, operatingMode, fanSpeed, temperature, swingV);
}

// Send the Samsung code

void SamsungAQVHeatpumpIR::sendSamsung(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV)
{
  uint8_t SamsungTemplate[] = { 0x02, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,   // Header part
                                0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,   // Always the same data on POWER messages
                                0x01, 0x00, 0xFE, 0x71, 0x00, 0x00, 0x00 }; // The actual data is in this part, on uint8_ts 14-20

  uint8_t SamsungChecksum = 0;

  // Set the power mode on the template message, also add the first part checksum
  SamsungTemplate[6] = powerMode;
  if ( powerMode == SAMSUNG_AIRCON1_MODE_ON ) {
    SamsungTemplate[1] = 0x92;
  } else {
    SamsungTemplate[1] = 0xB2;
  }

  SamsungTemplate[20] = powerMode;

  // Set the fan speed and the operating mode on the template message
  SamsungTemplate[19] = operatingMode | fanSpeed;

  // Set the temperature on the template message
  SamsungTemplate[18] = (temperature - 16) << 4;

  // Set the vertical swing mode on the template message
  SamsungTemplate[16] = swingV;

  // Calculate the byte 15 checksum
  // Count the number of ONE bits on message uint8_ts 15-20
  for (uint8_t j=15; j<21; j++) {
    uint8_t Samsungbyte = SamsungTemplate[j];
    for (uint8_t i=0; i<8; i++) {
      if ( (Samsungbyte & 0x01) == 0x01 ) {
        SamsungChecksum++;
      }
      Samsungbyte >>= 1;
    }
  }

  // Transform the number of ONE bits to the actual checksum
  SamsungChecksum = 32 - SamsungChecksum;
  SamsungChecksum <<= 4;
  SamsungChecksum += 0x02;

  SamsungTemplate[15] = SamsungChecksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload header part
  for (int i=0; i<7; i++) {
    IR.sendIRbyte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(SAMSUNG_AIRCON1_MSG_SPACE);

  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload power message part
  for (int i=7; i<14; i++) {
    IR.sendIRbyte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(SAMSUNG_AIRCON1_MSG_SPACE);

  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload data message part
  for (int i=14; i<21; i++) {
    IR.sendIRbyte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(0);
}

// Samsung FJM (RJ040F2HXEA / MH026FNEA) heatpump control (remote control P/N ARH-465)

void SamsungFJMHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, false);
}

void SamsungFJMHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboModeCmd)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = SAMSUNG_AIRCON1_MODE_ON;
  uint8_t operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;
  uint8_t swingV = SAMSUNG_AIRCON2_VS_AUTO;

  // See also https://github.com/wisskar/souliss/blob/master/extras/SamsungMH026FB.cpp

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = POWER_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = SAMSUNG_AIRCON1_MODE_AUTO;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in AUTO mode
        break;
      case MODE_HEAT:
        operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = SAMSUNG_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = SAMSUNG_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = SAMSUNG_AIRCON1_MODE_FAN;
        temperatureCmd = 24; // Temperature is always 24 in FAN mode
        if ( fanSpeedCmd == FAN_AUTO ) {
          fanSpeedCmd = FAN_1; // Fan speed cannot be 'AUTO' in FAN mode
        }
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = SAMSUNG_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = SAMSUNG_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = SAMSUNG_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = SAMSUNG_AIRCON2_FAN4;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 28)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = SAMSUNG_AIRCON2_VS_SWING;
      break;
  }

  sendSamsung(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, turboModeCmd);
}

// Send the Samsung code

void SamsungFJMHeatpumpIR::sendSamsung(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, bool turboMode)
{
  static const uint8_t samsungOffCode[] PROGMEM = { 0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
                                                    0x01, 0x72, 0x0F, 0x00, 0x90, 0xD0, 0x01,
                                                    0x01, 0x02, 0xFF, 0x01, 0x60, 0x4B, 0xC0 };

  static const uint8_t samsungHeader[] PROGMEM = { 0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0 };

//  uint8_t samsungTemplate[] = { 0x01, 0x00, 0x0E, 0x01, 0x00, 0x00, 0xF0 }; //Orig
  uint8_t samsungTemplate[] = { 0x01, 0x00, 0x0F, 0x01, 0x00, 0x00, 0xF0 };  
  //                               0     1     2     3     4     5     6
  //                               7     8     9    10    11    12    13 <- byte in whole sequence

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(SAMSUNG_AIRCON2_HDR_MARK);
  IR.space(SAMSUNG_AIRCON2_HDR_SPACE);

  if (powerMode == POWER_OFF)
  {
    for (size_t i=0; i<sizeof(samsungOffCode); i++) {
      IR.sendIRbyte(pgm_read_byte(&samsungOffCode[i]), SAMSUNG_AIRCON2_BIT_MARK, SAMSUNG_AIRCON2_ZERO_SPACE, SAMSUNG_AIRCON2_ONE_SPACE);

      if (i == 6 || i == 13)
      {
        IR.mark(SAMSUNG_AIRCON2_BIT_MARK);
        IR.space(SAMSUNG_AIRCON2_ONE_SPACE);
        IR.mark(SAMSUNG_AIRCON2_HDR_MARK);
        IR.space(SAMSUNG_AIRCON2_HDR_SPACE);
      }
    }
  }
  else
  {
    for (size_t i=0; i<sizeof(samsungHeader); i++) {
      IR.sendIRbyte(pgm_read_byte(&samsungHeader[i]), SAMSUNG_AIRCON2_BIT_MARK, SAMSUNG_AIRCON2_ZERO_SPACE, SAMSUNG_AIRCON2_ONE_SPACE);
    }

    IR.mark(SAMSUNG_AIRCON2_BIT_MARK);
    IR.space(SAMSUNG_AIRCON2_ONE_SPACE);
    IR.mark(SAMSUNG_AIRCON2_HDR_MARK);
    IR.space(SAMSUNG_AIRCON2_HDR_SPACE);

    // The actual data is in the last 7 bytes
    samsungTemplate[2] |= swingV;
    samsungTemplate[4] = (temperature - 16) << 4;
    samsungTemplate[5] = operatingMode | fanSpeed;
    
    if (turboMode)
    {
      samsungTemplate[3] |= SAMSUNG_AIRCON2_TURBO;
    }

    // Checksum calculation
    uint8_t checksum = 0x00;

    for (uint8_t j=2; j<6; j++) {
      uint8_t samsungByte = samsungTemplate[j];

      if (j == 2)
      {
        samsungByte &= 0b11111110;
      }

      for (uint8_t i=0; i<8; i++) {
        if ( (samsungByte & 0x01) == 0x01 ) {
          checksum++;
        }
        samsungByte >>= 1;
      }
    }

    checksum = 28 - checksum;
    checksum <<= 4;
    checksum |= 0x02;

    samsungTemplate[1] = checksum;

    for (size_t i=0; i<sizeof(samsungTemplate); i++) {
      IR.sendIRbyte(samsungTemplate[i], SAMSUNG_AIRCON2_BIT_MARK, SAMSUNG_AIRCON2_ZERO_SPACE, SAMSUNG_AIRCON2_ONE_SPACE);
    }
  }

  // End mark
  IR.mark(SAMSUNG_AIRCON2_BIT_MARK);
  IR.space(0);
}


