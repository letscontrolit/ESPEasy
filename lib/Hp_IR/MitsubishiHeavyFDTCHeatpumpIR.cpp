#include <MitsubishiHeavyFDTCHeatpumpIR.h>

MitsubishiHeavyFDTCHeatpumpIR::MitsubishiHeavyFDTCHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_heavy_fdtc";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_heavy_fdtc\",\"dn\":\"Mitsubishi Heavy FDTC\",\"mT\":18,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

void MitsubishiHeavyFDTCHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode
  uint8_t powerMode     = MITSUBISHI_HEAVY_FDTC_MODE_ON;
  uint8_t operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_AUTO;
  uint8_t fanSpeed      = MITSUBISHI_HEAVY_FDTC_FAN1;
  uint8_t temperature   = 21;
  uint8_t swingV        = MITSUBISHI_HEAVY_FDTC_VS_UP;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = MITSUBISHI_HEAVY_FDTC_MODE_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MITSUBISHI_HEAVY_FDTC_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_1:
      fanSpeed = MITSUBISHI_HEAVY_FDTC_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHI_HEAVY_FDTC_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHI_HEAVY_FDTC_FAN3;
      break;
  }


  if ( temperatureCmd > 17 && temperatureCmd < 31)
  {
    temperature = (temperatureCmd - 16) & 0x0F;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_SWING;
      break;
    case VDIR_UP:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_MUP;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHI_HEAVY_FDTC_VS_DOWN;
      break;
  }

  sendMitsubishiHeavyFDTC(IR, powerMode, operatingMode, fanSpeed, temperature, swingV);
}


void MitsubishiHeavyFDTCHeatpumpIR::sendMitsubishiHeavyFDTC(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV)
{
  //uint8_t MitsubishiHeavyFDTCTemplate[] = { 0x0B, 0x00, 0x00, 0x40, 0xF4, 0xFF, 0xFF, 0xBF}; //first byte could be 0B
  uint8_t MitsubishiHeavyFDTCTemplate[] = { 0x0A, 0x00, 0x00, 0x40, 0xF4, 0xFF, 0xFF, 0xBF};
  //                                           0     1     2     3     4     5     6     7

  // Vertical air flow + fan speed
  MitsubishiHeavyFDTCTemplate[1] |= (swingV & 0b01000000) | fanSpeed;

  // Power state + operating mode + temperature
  MitsubishiHeavyFDTCTemplate[2] |= operatingMode | powerMode | temperature;

  // Vertical air flow
  MitsubishiHeavyFDTCTemplate[3] |= (swingV & 0b00110000);

  // There is no checksum, but some bytes are inverted
  MitsubishiHeavyFDTCTemplate[4] = ~MitsubishiHeavyFDTCTemplate[0];
  MitsubishiHeavyFDTCTemplate[5] = ~MitsubishiHeavyFDTCTemplate[1];
  MitsubishiHeavyFDTCTemplate[6] = ~MitsubishiHeavyFDTCTemplate[2];
  MitsubishiHeavyFDTCTemplate[7] = ~MitsubishiHeavyFDTCTemplate[3];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHI_HEAVY_FDTC_HDR_MARK);
  IR.space(MITSUBISHI_HEAVY_FDTC_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(MitsubishiHeavyFDTCTemplate); i++) {
    IR.sendIRbyte(MitsubishiHeavyFDTCTemplate[i], MITSUBISHI_HEAVY_FDTC_BIT_MARK, MITSUBISHI_HEAVY_FDTC_ZERO_SPACE, MITSUBISHI_HEAVY_FDTC_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHI_HEAVY_FDTC_BIT_MARK);  
  IR.space(MITSUBISHI_HEAVY_FDTC_HDR_SPACE);
  
  IR.mark(MITSUBISHI_HEAVY_FDTC_BIT_MARK); 
  IR.space(0);
}
