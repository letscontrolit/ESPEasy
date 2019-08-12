#include <MitsubishiSEZKDXXHeatpumpIR.h>

MitsubishiSEZKDXXHeatpumpIR::MitsubishiSEZKDXXHeatpumpIR() : HeatpumpIR()
{
  static const char model[]  PROGMEM = "mitsubishi_sez";
  static const char info[]   PROGMEM = "{\"mdl\":\"mitsubishi_sez\",\"dn\":\"Mitsubishi SEZ\",\"mT\":17,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}


void MitsubishiSEZKDXXHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{

  // Sensible defaults for the heat pump mode
  uint8_t powerMode     = MITSUBISHISEZ_AIRCON1_MODE_ON;
  uint8_t operatingMode = MITSUBISHISEZ_AIRCON1_MODE_AUTO;
  uint8_t fanSpeed      = MITSUBISHISEZ_AIRCON1_FAN1;
  uint8_t temperature   = 23;
  uint8_t swingV        = 0x00;
  uint8_t swingH        = 0x00;

  switch (powerModeCmd)
  {
    case POWER_OFF:
      // OFF is a special case
      powerMode = MITSUBISHISEZ_AIRCON1_MODE_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHISEZ_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHISEZ_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHISEZ_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHISEZ_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MITSUBISHISEZ_AIRCON1_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHISEZ_AIRCON1_FAN1;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHISEZ_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHISEZ_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHISEZ_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 16 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

   sendMitsubishiSEZKDXX(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}


void MitsubishiSEZKDXXHeatpumpIR::sendMitsubishiSEZKDXX(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t MitsubishiTemplate[] = { 0x23, 0xCB, 0x26, 0x21, 0x00, 0x40, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //                                  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16


  // Set the operatingmode on the template message
  MitsubishiTemplate[5] = powerMode;

  // Set the temperature and operating mode on the template message
  MitsubishiTemplate[6] = ((temperature - 16) << 4) | operatingMode;

  // Set the fan speed and vertical air direction on the template message
  MitsubishiTemplate[7] = fanSpeed;

   //Check Byte
  MitsubishiTemplate[11] = ~MitsubishiTemplate[5];
  MitsubishiTemplate[12] = ~MitsubishiTemplate[6];
  MitsubishiTemplate[13] = ~MitsubishiTemplate[7];
  MitsubishiTemplate[14] = ~MitsubishiTemplate[8];
  MitsubishiTemplate[15] = ~MitsubishiTemplate[9];
  MitsubishiTemplate[16] = ~MitsubishiTemplate[10];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHISEZ_AIRCON1_HDR_MARK);
  IR.space(MITSUBISHISEZ_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<sizeof(MitsubishiTemplate); i++) {
    IR.sendIRbyte(MitsubishiTemplate[i], MITSUBISHISEZ_AIRCON1_BIT_MARK, MITSUBISHISEZ_AIRCON1_ZERO_SPACE, MITSUBISHISEZ_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHISEZ_AIRCON1_BIT_MARK);
  IR.space(0);
}


