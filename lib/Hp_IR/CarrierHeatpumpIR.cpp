#include <CarrierHeatpumpIR.h>

// This is a protected method, i.e. generic Carrier instances cannot be created
CarrierHeatpumpIR::CarrierHeatpumpIR() : HeatpumpIR()
{
}

void CarrierHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
}

// The different models just set the model accordingly
CarrierNQVHeatpumpIR::CarrierNQVHeatpumpIR() : CarrierHeatpumpIR()
{
  static const char model[] PROGMEM = "carrier_nqv";
  static const char info[]  PROGMEM = "{\"mdl\":\"carrier_nqv\",\"dn\":\"Carrier NQV\",\"mT\":17,\"xT\":30,\"fs\":6}";

  _model = model;
  _info = info;
}

CarrierMCAHeatpumpIR::CarrierMCAHeatpumpIR() : CarrierHeatpumpIR()
{
  static const char model[] PROGMEM = "carrier_mca";
  static const char info[]  PROGMEM = "{\"mdl\":\"carrier_mca\",\"dn\":\"Carrier MCA\",\"mT\":17,\"xT\":30,\"fs\":4}";

  _model = model;
  _info = info;
  _carrierModel = MODEL_CARRIER_MCA;
}

Qlima1HeatpumpIR::Qlima1HeatpumpIR() : CarrierMCAHeatpumpIR()
{
  static const char model[] PROGMEM = "qlima_1";
  static const char info[]  PROGMEM = "{\"mdl\":\"qlima_1\",\"dn\":\"Qlima #1\",\"mT\":17,\"xT\":30,\"fs\":4, \"maint\":[10]}";

  _model = model;
  _info = info;

  _carrierModel = MODEL_QLIMA_1;
}

Qlima2HeatpumpIR::Qlima2HeatpumpIR() : CarrierMCAHeatpumpIR()
{
  static const char model[] PROGMEM = "qlima_2";
  static const char info[]  PROGMEM = "{\"mdl\":\"qlima_2\",\"dn\":\"Qlima #2\",\"mT\":17,\"xT\":30,\"fs\":4, \"maint\":[10]}";

  _model = model;
  _info = info;

  _carrierModel = MODEL_QLIMA_2;
}


void CarrierNQVHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = CARRIER_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = CARRIER_AIRCON1_FAN_AUTO;
  uint8_t temperature = 23;

  if (powerModeCmd == POWER_OFF)
  {
    operatingMode = CARRIER_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = CARRIER_AIRCON1_MODE_AUTO;
        break;
      case MODE_HEAT:
        operatingMode = CARRIER_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = CARRIER_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = CARRIER_AIRCON1_MODE_DRY;
		    fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = CARRIER_AIRCON1_MODE_FAN;
        temperatureCmd = 22; // Temperature is always 22 in FAN mode
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = CARRIER_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = CARRIER_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = CARRIER_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = CARRIER_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = CARRIER_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = CARRIER_AIRCON1_FAN5;
      break;
  }

  if (temperatureCmd > 16 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  sendCarrier(IR, operatingMode, fanSpeed, temperature);
}

// Send the Carrier code
// Carrier has the LSB and MSB in different format than Panasonic

void CarrierNQVHeatpumpIR::sendCarrier(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature)
{
  uint8_t sendBuffer[] = { 0x4f, 0xb0, 0xc0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00 }; // The data is on the last four bytes

  static const uint8_t temperatures[] PROGMEM = { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b };
  uint8_t checksum = 0;

  // PROGMEM arrays cannot be addressed directly, see http://forum.arduino.cc/index.php?topic=106603.0
  sendBuffer[5] = pgm_read_byte(&(temperatures[(temperature-17)]));

  sendBuffer[6] = operatingMode | fanSpeed;

  // Checksum

  for (int i=0; i<8; i++) {
    checksum += IR.bitReverse(sendBuffer[i]);
  }

  // There's something really strange with the checksum calculation...
  // With these many of the codes matchs with the code from the real Carrier remote
  // Still certain temperatures do not work with fan speeds 1, 2 or 5

  switch (sendBuffer[6] & 0xF0) {
  case 0x00: // MODE_AUTO - certain temperature / fan speed combinations do not work
    checksum += 0x02;
    switch (sendBuffer[6] & 0x0F) {
      case 0x02: // FAN1
      case 0x03: // FAN5
      case 0x06: // FAN2
        checksum += 0x80;
        break;
    }
    break;
  case 0x40: // MODE_DRY - all settings should work
    checksum += 0x02;
    break;
  case 0xC0: // MODE_HEAT - certain temperature / fan speed combinations do not work
    switch (sendBuffer[6] & 0x0F) {
      case 0x05: // FAN4
      case 0x06: // FAN2
        checksum += 0xC0;
        break;
    }
    break;
  case 0x20: // MODE_FAN - all settings should work
    checksum += 0x02;
    switch (sendBuffer[6] & 0x0F) {
      case 0x02: // FAN1
      case 0x03: // FAN5
      case 0x06: // FAN2
        checksum += 0x80;
        break;
    }
    break;
  }

  sendBuffer[8] = IR.bitReverse(checksum);

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(CARRIER_AIRCON1_HDR_MARK);
  IR.space(CARRIER_AIRCON1_HDR_SPACE);

  // Payload
  for (size_t i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRbyte(sendBuffer[i], CARRIER_AIRCON1_BIT_MARK, CARRIER_AIRCON1_ZERO_SPACE, CARRIER_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(CARRIER_AIRCON1_BIT_MARK);
  IR.space(CARRIER_AIRCON1_MSG_SPACE);

  IR.mark(CARRIER_AIRCON1_HDR_MARK);
  IR.space(CARRIER_AIRCON1_HDR_SPACE);

  // Payload again
  for (size_t i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRbyte(sendBuffer[i], CARRIER_AIRCON1_BIT_MARK, CARRIER_AIRCON1_ZERO_SPACE, CARRIER_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(CARRIER_AIRCON1_BIT_MARK);
  IR.space(0);
}

void CarrierMCAHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, false);
}


void CarrierMCAHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the A/C mode

  uint8_t powerMode = CARRIER_AIRCON2_MODE_ON;
  uint8_t operatingMode = CARRIER_AIRCON2_MODE_COOL;
  uint8_t fanSpeed = CARRIER_AIRCON2_FAN_AUTO;
  uint8_t temperature = 23;
  bool maintenanceMode = false;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = CARRIER_AIRCON2_MODE_OFF;
    operatingMode = CARRIER_AIRCON2_MODE_COOL;
    fanSpeed = CARRIER_AIRCON2_FAN_OFF;
    temperature = 31;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = CARRIER_AIRCON2_MODE_AUTO;
		    fanSpeed = CARRIER_AIRCON2_FAN_DRY_AUTO; // Fan speed is always 'AUTO' in AUTO mode
        break;
      case MODE_COOL:
        operatingMode = CARRIER_AIRCON2_MODE_COOL;
        break;
      case MODE_HEAT:
        operatingMode = CARRIER_AIRCON2_MODE_HEAT;
        break;
      case MODE_DRY:
        operatingMode = CARRIER_AIRCON2_MODE_DRY;
		    fanSpeed = CARRIER_AIRCON2_FAN_DRY_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = CARRIER_AIRCON2_MODE_FAN;
        temperature = 31; // Temperature is always 31 in FAN mode
        break;
      case MODE_MAINT:
        maintenanceMode = true;
        break;
    }

    if (operatingModeCmd != MODE_AUTO
        && operatingModeCmd != MODE_DRY)
      {
        switch (fanSpeedCmd)
        {
          case FAN_AUTO:
            fanSpeed = CARRIER_AIRCON2_FAN_AUTO;
            break;
          case FAN_1:
            fanSpeed = CARRIER_AIRCON2_FAN1;
            break;
          case FAN_2:
            fanSpeed = CARRIER_AIRCON2_FAN2;
            break;
          case FAN_3:
            fanSpeed = CARRIER_AIRCON2_FAN3;
            break;
        }
      }

    if ( temperatureCmd > 16 && temperatureCmd < 31
         && operatingModeCmd != MODE_FAN)
    {
      temperature = temperatureCmd;
    }
  }

  sendCarrier(IR, powerMode, operatingMode, fanSpeed, temperature, maintenanceMode, turboMode);
}

void CarrierMCAHeatpumpIR::sendCarrier(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, bool maintenanceMode, bool turboMode)
{
  uint8_t sendBuffer[] = { 0x4D, 0xB2, 0xD8, 0x00, 0x00, 0x00 };
  static const uint8_t sendBufferMaintenance1[] PROGMEM = { 0xAD, 0x52, 0xAF, 0x50, 0xB5, 0x4A };
  static const uint8_t sendBufferMaintenance2[] PROGMEM = { 0xAD, 0x52, 0xAF, 0x50, 0x55, 0xAA };
  static const uint8_t sendBufferTurbo[]        PROGMEM = { 0xAD, 0x52, 0xAF, 0x50, 0x45, 0xBA };

  static const uint8_t temperatures[] PROGMEM = { 0, 8, 12, 4, 6, 14, 10, 2, 3, 11, 9, 1, 5, 13, 7 };

  sendBuffer[2] |= powerMode | fanSpeed;

  // PROGMEM arrays cannot be addressed directly, see http://forum.arduino.cc/index.php?topic=106603.0
  sendBuffer[4] |= operatingMode | pgm_read_byte(&(temperatures[(temperature-17)]));

  // Checksums
  sendBuffer[3] = ~sendBuffer[2];
  sendBuffer[5] = ~sendBuffer[4];

  // Maintenance or turbo mode overide
  if (maintenanceMode && _carrierModel == MODEL_QLIMA_1)
  {
    memcpy_P(sendBuffer, sendBufferMaintenance1, sizeof(sendBufferMaintenance1));    
  }
  if (maintenanceMode && _carrierModel == MODEL_QLIMA_2)
  {
    memcpy_P(sendBuffer, sendBufferMaintenance2, sizeof(sendBufferMaintenance2));    
  }  
  if (turboMode && (_carrierModel == MODEL_QLIMA_1 || _carrierModel == MODEL_QLIMA_2))
  {
    memcpy_P(sendBuffer, sendBufferTurbo, sizeof(sendBufferTurbo));
  }
  
  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(CARRIER_AIRCON2_HDR_MARK);
  IR.space(CARRIER_AIRCON2_HDR_SPACE);

  // Payload
  for (size_t i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRbyte(sendBuffer[i], CARRIER_AIRCON2_BIT_MARK, CARRIER_AIRCON2_ZERO_SPACE, CARRIER_AIRCON2_ONE_SPACE);
  }

  // New header
  IR.mark(CARRIER_AIRCON2_BIT_MARK);
  IR.space(CARRIER_AIRCON2_HDR_SPACE);
  IR.mark(CARRIER_AIRCON2_HDR_MARK);
  IR.space(CARRIER_AIRCON2_HDR_SPACE);

  // Payload again
  for (size_t i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRbyte(sendBuffer[i], CARRIER_AIRCON2_BIT_MARK, CARRIER_AIRCON2_ZERO_SPACE, CARRIER_AIRCON2_ONE_SPACE);
  }

  // End mark
  IR.mark(CARRIER_AIRCON2_BIT_MARK);
  IR.space(0);
}
