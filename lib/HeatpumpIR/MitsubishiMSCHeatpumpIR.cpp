#include <MitsubishiMSCHeatpumpIR.h>


MitsubishiMSCHeatpumpIR::MitsubishiMSCHeatpumpIR() : HeatpumpIR()
{
  static const char model[]  PROGMEM = "mitsubishi_msc";
  static const char info[]   PROGMEM = "{\"mdl\":\"mitsubishi_msc\",\"dn\":\"Mitsubishi MSC\",\"mT\":17,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}


void MitsubishiMSCHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{

  // Sensible defaults for the heat pump mode
  uint8_t powerMode     = MITSUBISHIMSC_AIRCON1_MODE_ON;
  uint8_t operatingMode = MITSUBISHIMSC_AIRCON1_MODE_AUTO;
  uint8_t fanSpeed      = MITSUBISHIMSC_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = MITSUBISHIMSC_AIRCON1_VS_AUTO;
  uint8_t swingH        = 0x00;

  switch (powerModeCmd)
  {
    case POWER_OFF:
      powerMode = MITSUBISHIMSC_AIRCON1_MODE_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHIMSC_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHIMSC_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHIMSC_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHIMSC_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MITSUBISHIMSC_AIRCON1_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHIMSC_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHIMSC_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHIMSC_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHIMSC_AIRCON1_FAN3;
      break;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = MITSUBISHIMSC_AIRCON1_VS_SWING;
      break;
    case VDIR_AUTO:
      swingV = MITSUBISHIMSC_AIRCON1_VS_AUTO;
      break;
    case VDIR_UP:
      swingV = MITSUBISHIMSC_AIRCON1_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHIMSC_AIRCON1_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHIMSC_AIRCON1_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHIMSC_AIRCON1_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHIMSC_AIRCON1_VS_DOWN;
      break;
  }

  if ( temperatureCmd >= 16 && temperatureCmd <= 31)
  {
    temperature = temperatureCmd;
  }

   sendMitsubishiMSC(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}


void MitsubishiMSCHeatpumpIR::sendMitsubishiMSC(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t MitsubishiTemplate[] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //                                  0     1     2     3     4     5     6     7     8     9    10    11    12    13

  uint8_t templateSize = sizeof(MitsubishiTemplate);
  uint8_t checksum = 0x00;

  // Set the power mode on the template message
  MitsubishiTemplate[5] = powerMode;

  // Set the operating mode on the template message
  MitsubishiTemplate[6] = operatingMode;

  // Set the temperature on the template message
  MitsubishiTemplate[7] = 31 - temperature;

  // Set the fan speed and vertical air direction on the template message
  MitsubishiTemplate[8] = fanSpeed | swingV;

  // Checksum

  for (int i=0; i<templateSize; i++) {
    checksum += MitsubishiTemplate[i];
  }

  MitsubishiTemplate[templateSize - 1] = checksum;

#ifdef IR_DEBUG_PACKET
  char pbyte[16];

  for (int i=0; i<templateSize; i++) {
    checksum += MitsubishiTemplate[i];
    sprintf_P(pbyte, PSTR(",%02x"),(int) MitsubishiTemplate[i]);
    Serial.print(pbyte);
  }

  Serial.println("");
#endif

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHIMSC_AIRCON1_HDR_MARK);
  IR.space(MITSUBISHIMSC_AIRCON1_HDR_SPACE);

  // Data
  for (unsigned int i=0; i<templateSize; i++) {
    IR.sendIRbyte(MitsubishiTemplate[i], MITSUBISHIMSC_AIRCON1_BIT_MARK, MITSUBISHIMSC_AIRCON1_ZERO_SPACE, MITSUBISHIMSC_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHIMSC_AIRCON1_BIT_MARK);
  IR.space(0);
}