#include <MitsubishiHeavyHeatpumpIR.h>

// These are protected methods, i.e. generic MitsubishiHeavy instances cannot be created directly
MitsubishiHeavyHeatpumpIR::MitsubishiHeavyHeatpumpIR() : HeatpumpIR()
{
}


// The different models just set the model accordingly

MitsubishiHeavyZJHeatpumpIR::MitsubishiHeavyZJHeatpumpIR() : MitsubishiHeavyHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_heavy_zj";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_heavy_zj\",\"dn\":\"Mitsubishi Heavy ZJ\",\"mT\":18,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHIHEAVY_ZJ;
}

MitsubishiHeavyZMHeatpumpIR::MitsubishiHeavyZMHeatpumpIR() : MitsubishiHeavyHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_heavy_zm";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_heavy_zm\",\"dn\":\"Mitsubishi Heavy ZM\",\"mT\":18,\"xT\":30,\"fs\":4}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHIHEAVY_ZM;
}

MitsubishiHeavyZMPHeatpumpIR::MitsubishiHeavyZMPHeatpumpIR() : MitsubishiHeavyHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_heavy_zmp";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_heavy_zmp\",\"dn\":\"Mitsubishi Heavy ZMP\",\"mT\":18,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHIHEAVY_ZMP;
}

void MitsubishiHeavyHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, true, false, false);
}

// This is a virtual dummy method, i.e. MitsubishiHeavyZJHeatpumpIR::send or MitsubishiHeavyZMHeatpumpIR::send is called instead
void MitsubishiHeavyHeatpumpIR::send(IRSender&, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool, bool, bool)
{
}


void MitsubishiHeavyZJHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = MITSUBISHI_HEAVY_MODE_ON;
  uint8_t operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
  uint8_t fanSpeed      = MITSUBISHI_HEAVY_ZJ_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = MITSUBISHI_HEAVY_ZJ_VS_STOP;
  uint8_t swingH        = MITSUBISHI_HEAVY_ZJ_HS_STOP;
  uint8_t cleanMode     = MITSUBISHI_HEAVY_ZJ_CLEAN_OFF;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = MITSUBISHI_HEAVY_MODE_OFF;
  }

  if (cleanModeCmd && powerModeCmd == POWER_OFF && (operatingModeCmd == MODE_AUTO || operatingModeCmd == MODE_COOL || operatingModeCmd == MODE_DRY))
  {
    powerMode = MITSUBISHI_HEAVY_MODE_ON;
    cleanMode = MITSUBISHI_HEAVY_CLEAN_ON;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHI_HEAVY_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHI_HEAVY_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHI_HEAVY_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MITSUBISHI_HEAVY_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHI_HEAVY_ZJ_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHI_HEAVY_ZJ_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHI_HEAVY_ZJ_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHI_HEAVY_ZJ_FAN3;
      break;
  }

  if (silentModeCmd)
  {
  	// Silent mode doesn't exist on ZJ model, use ECONO mode instead
    fanSpeed = MITSUBISHI_HEAVY_ZJ_SILENT_ON;
  }

  if ( temperatureCmd > 17 && temperatureCmd < 31)
  {
    temperature = (~((temperatureCmd - 17) << 4)) & 0xF0;
  }

  switch (swingVCmd)
  {
    case VDIR_MANUAL:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_STOP;
      break;
    case VDIR_SWING:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_SWING;
      break;
    case VDIR_UP:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHI_HEAVY_ZJ_VS_DOWN;
      break;
  }

  switch (swingHCmd)
  {
    case HDIR_MANUAL:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_STOP;
      break;
    case HDIR_SWING:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_SWING;
      break;
    case HDIR_MIDDLE:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_MIDDLE;
      break;
    case HDIR_LEFT:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_LEFT;
      break;
    case HDIR_MLEFT:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_MLEFT;
      break;
    case HDIR_RIGHT:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_RIGHT;
      break;
    case HDIR_MRIGHT:
      swingH = MITSUBISHI_HEAVY_ZJ_HS_MRIGHT;
      break;
  }

  if (_3DAutoCmd == true && (operatingModeCmd == MODE_AUTO || operatingModeCmd == MODE_COOL || operatingModeCmd == MODE_HEAT))
  {
      swingH = MITSUBISHI_HEAVY_ZJ_HS_3DAUTO;
  }
  sendMitsubishiHeavy(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH, cleanMode);
}


void MitsubishiHeavyZMHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = MITSUBISHI_HEAVY_MODE_ON;
  uint8_t operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
  uint8_t fanSpeed      = MITSUBISHI_HEAVY_ZM_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = MITSUBISHI_HEAVY_ZM_VS_STOP;
  uint8_t swingH        = MITSUBISHI_HEAVY_ZM_HS_STOP;
  uint8_t cleanMode     = MITSUBISHI_HEAVY_ZM_CLEAN_OFF;
  uint8_t _3DAuto       = MITSUBISHI_HEAVY_ZM_3DAUTO_OFF;
  uint8_t silentMode    = MITSUBISHI_HEAVY_ZM_SILENT_OFF;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = MITSUBISHI_HEAVY_MODE_OFF;
  }

  if (cleanModeCmd && powerModeCmd == POWER_OFF && (operatingModeCmd == MODE_AUTO || operatingModeCmd == MODE_COOL || operatingModeCmd == MODE_DRY))
  {
    powerMode = MITSUBISHI_HEAVY_MODE_ON;
    cleanMode = MITSUBISHI_HEAVY_CLEAN_ON;
  }

  if (silentModeCmd && !(operatingModeCmd == MODE_DRY || operatingModeCmd == MODE_FAN))
  {
    silentMode = MITSUBISHI_HEAVY_ZM_SILENT_ON;
  }

  if (_3DAutoCmd && !(operatingModeCmd == MODE_DRY || operatingModeCmd == MODE_FAN))
  {
    _3DAuto = MITSUBISHI_HEAVY_ZM_3DAUTO_ON;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHI_HEAVY_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHI_HEAVY_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHI_HEAVY_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode = MITSUBISHI_HEAVY_MODE_FAN;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHI_HEAVY_ZM_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHI_HEAVY_ZM_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHI_HEAVY_ZM_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHI_HEAVY_ZM_FAN3;
      break;
    case FAN_4:
      fanSpeed = MITSUBISHI_HEAVY_ZM_FAN4;
      break;
  }

  if ( temperatureCmd > 17 && temperatureCmd < 31)
  {
    temperature = (~(temperatureCmd - 17) & 0x0F);
  }

  switch (swingVCmd)
  {
    case VDIR_MANUAL:
      swingV = MITSUBISHI_HEAVY_ZM_VS_STOP;
      break;
    case VDIR_SWING:
      swingV = MITSUBISHI_HEAVY_ZM_VS_SWING;
      break;
    case VDIR_UP:
      swingV = MITSUBISHI_HEAVY_ZM_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHI_HEAVY_ZM_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHI_HEAVY_ZM_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHI_HEAVY_ZM_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHI_HEAVY_ZM_VS_DOWN;
      break;
  }

  switch (swingHCmd)
  {
    case HDIR_MANUAL:
      swingH = MITSUBISHI_HEAVY_ZM_HS_STOP;
      break;
    case HDIR_SWING:
      swingH = MITSUBISHI_HEAVY_ZM_HS_SWING;
      break;
    case HDIR_MIDDLE:
      swingH = MITSUBISHI_HEAVY_ZM_HS_MIDDLE;
      break;
    case HDIR_LEFT:
      swingH = MITSUBISHI_HEAVY_ZM_HS_LEFT;
      break;
    case HDIR_MLEFT:
      swingH = MITSUBISHI_HEAVY_ZM_HS_MLEFT;
      break;
    case HDIR_RIGHT:
      swingH = MITSUBISHI_HEAVY_ZM_HS_RIGHT;
      break;
    case HDIR_MRIGHT:
      swingH = MITSUBISHI_HEAVY_ZM_HS_MRIGHT;
      break;
  }

  sendMitsubishiHeavy(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH, cleanMode, silentMode, _3DAuto);
}

void MitsubishiHeavyZMPHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = MITSUBISHI_HEAVY_MODE_ON;
  uint8_t operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
  uint8_t fanSpeed      = MITSUBISHI_HEAVY_ZMP_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = MITSUBISHI_HEAVY_ZMP_VS_STOP;
  uint8_t swingH        = MITSUBISHI_HEAVY_ZMP_HS_STOP;
  uint8_t cleanMode     = MITSUBISHI_HEAVY_ZMP_CLEAN_OFF;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = MITSUBISHI_HEAVY_MODE_OFF;
  }

  if (operatingModeCmd == MODE_MAINT && powerModeCmd == POWER_OFF)
  {
    powerMode = MITSUBISHI_HEAVY_MODE_ON;
    cleanMode = MITSUBISHI_HEAVY_ZMP_CLEAN_ON;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode = MITSUBISHI_HEAVY_MODE_AUTO;
      //In MODE_AUTO we need to handle temperature differently. It can range from -6 to +6
      temperature = 0x80 - (0x10*temperatureCmd);
      break;
    case MODE_HEAT:
      operatingMode = MITSUBISHI_HEAVY_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode = MITSUBISHI_HEAVY_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode = MITSUBISHI_HEAVY_MODE_DRY;
      break;
    case MODE_FAN:
      //Fan mode has no temperature setting
      //ZMP model uses different code for fan mode.
      operatingMode = MITSUBISHI_HEAVY_ZMP_MODE_FAN;
      temperature = 0;
      break;
    case MODE_MAINT:
      //Specify maintenance mode to activate clean mode
      operatingMode = MITSUBISHI_HEAVY_ZMP_MODE_MAINT;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHI_HEAVY_ZMP_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHI_HEAVY_ZMP_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHI_HEAVY_ZMP_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHI_HEAVY_ZMP_FAN3;
      break;
    case FAN_4: //Map FAN_4 to HiPower
      fanSpeed = MITSUBISHI_HEAVY_ZMP_HIPOWER;
      break;
    case FAN_5: //Map FAN_5 to Econo
      fanSpeed = MITSUBISHI_HEAVY_ZMP_ECONO;
      break;
  }

  if (silentModeCmd)
  {
  	// Silent mode doesn't exist on ZMP model, use ECONO mode instead
    fanSpeed = MITSUBISHI_HEAVY_ZMP_SILENT_ON;
  }
  
  if ( temperatureCmd > 17 && temperatureCmd < 31)
  {
    temperature = (~((temperatureCmd - 17) << 4)) & 0xF0;
  }

  switch (swingVCmd)
  {
    case VDIR_MANUAL:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_STOP;
      break;
    case VDIR_SWING:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_SWING;
      break;
    case VDIR_UP:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHI_HEAVY_ZMP_VS_DOWN;
      break;
  }

  /* ZMP model has no horizontal swing
  */

  Serial.println("Calling sendHeavy from ZMP with");
  Serial.print(F("PowerMode: "));
  Serial.println(powerMode);
  Serial.print(F("OperatingMode: "));
  Serial.println(operatingMode);
  Serial.print(F("FanSpeed: "));
  Serial.println(fanSpeed);
  Serial.print(F("Temperature: "));
  Serial.println(temperature);
  Serial.print(F("swingV: "));
  Serial.println(swingV);
  Serial.print(F("swingH: "));
  Serial.println(swingH);
  Serial.print(F("cleanMode: "));
  Serial.println(cleanMode);
  sendMitsubishiHeavy(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH, cleanMode);
}

void MitsubishiHeavyZJHeatpumpIR::sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode)
{
  uint8_t MitsubishiHeavyZJTemplate[] = { 0x52, 0xAE, 0xC3, 0x26, 0xD9, 0x11, 0x00, 0x07, 0x00, 0x00, 0x00 };
  //                                         0     1     2     3     4     5     6     7     8     9    10

  // Horizontal & vertical air flow + allergen + clean + 3D
  MitsubishiHeavyZJTemplate[5] |= swingH | (swingV & 0b00000010) | cleanMode;

  // Vertical air flow + fan speed
  MitsubishiHeavyZJTemplate[7] |= fanSpeed | (swingV & 0b00011000);

  // Power state + operating mode + temperature
  MitsubishiHeavyZJTemplate[9] |= operatingMode | powerMode | temperature;

  // There is no checksum, but some bytes are inverted
  MitsubishiHeavyZJTemplate[6] = ~MitsubishiHeavyZJTemplate[5];
  MitsubishiHeavyZJTemplate[8] = ~MitsubishiHeavyZJTemplate[7];
  MitsubishiHeavyZJTemplate[10] = ~MitsubishiHeavyZJTemplate[9];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHI_HEAVY_HDR_MARK);
  IR.space(MITSUBISHI_HEAVY_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(MitsubishiHeavyZJTemplate); i++) {
    IR.sendIRbyte(MitsubishiHeavyZJTemplate[i], MITSUBISHI_HEAVY_BIT_MARK, MITSUBISHI_HEAVY_ZERO_SPACE, MITSUBISHI_HEAVY_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHI_HEAVY_BIT_MARK);
  IR.space(0);
}

void MitsubishiHeavyZMHeatpumpIR::sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode, uint8_t silentMode, uint8_t _3DAuto)
{
  uint8_t MitsubishiHeavyZMTemplate[] = { 0x52, 0xAE, 0xC3, 0x1A, 0xE5, 0x90, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x0D, 0x00, 0x10, 0x00, 0xFF, 0x00, 0x7B, 0x00 };
  //                                         0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18

  // Power state + operating mode
  MitsubishiHeavyZMTemplate[5] |= operatingMode | powerMode | cleanMode;

  // Temperature
  MitsubishiHeavyZMTemplate[7] |= temperature;

  // Fan speed
  MitsubishiHeavyZMTemplate[9] |= fanSpeed;

  // Vertical air flow + 3D auto
  MitsubishiHeavyZMTemplate[11] |= swingV | _3DAuto;

  // Horizontal air flow
  MitsubishiHeavyZMTemplate[13] |= swingH | swingV;

  // Silent
  MitsubishiHeavyZMTemplate[15] |= silentMode;

  // There is no checksum, but some bytes are inverted
  MitsubishiHeavyZMTemplate[6] = ~MitsubishiHeavyZMTemplate[5];
  MitsubishiHeavyZMTemplate[8] = ~MitsubishiHeavyZMTemplate[7];
  MitsubishiHeavyZMTemplate[10] = ~MitsubishiHeavyZMTemplate[9];
  MitsubishiHeavyZMTemplate[12] = ~MitsubishiHeavyZMTemplate[11];
  MitsubishiHeavyZMTemplate[14] = ~MitsubishiHeavyZMTemplate[13];
  MitsubishiHeavyZMTemplate[16] = ~MitsubishiHeavyZMTemplate[15];
  MitsubishiHeavyZMTemplate[18] = ~MitsubishiHeavyZMTemplate[17];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHI_HEAVY_HDR_MARK);
  IR.space(MITSUBISHI_HEAVY_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(MitsubishiHeavyZMTemplate); i++) {
    IR.sendIRbyte(MitsubishiHeavyZMTemplate[i], MITSUBISHI_HEAVY_BIT_MARK, MITSUBISHI_HEAVY_ZERO_SPACE, MITSUBISHI_HEAVY_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHI_HEAVY_BIT_MARK);
  IR.space(0);
}

void MitsubishiHeavyZMPHeatpumpIR::sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode)
{
  uint8_t MitsubishiHeavyZMPTemplate[] = { 0x52, 0xAE, 0xC3, 0x26, 0xD9, 0x11, 0x00, 0x07, 0x00, 0x00, 0x00 };
  //                                         0     1     2     3     4     5     6     7     8     9    10

  // Horizontal & vertical air flow + allergen + clean + 3D
  MitsubishiHeavyZMPTemplate[5] |= swingH | (swingV & 0b00000010) | cleanMode;

  // Vertical air flow + fan speed
  MitsubishiHeavyZMPTemplate[7] |= fanSpeed | (swingV & 0b00011000);

  // Power state + operating mode + temperature
  MitsubishiHeavyZMPTemplate[9] |= operatingMode | powerMode | temperature;

  // There is no checksum, but some bytes are inverted
  MitsubishiHeavyZMPTemplate[6] = ~MitsubishiHeavyZMPTemplate[5];
  MitsubishiHeavyZMPTemplate[8] = ~MitsubishiHeavyZMPTemplate[7];
  MitsubishiHeavyZMPTemplate[10] = ~MitsubishiHeavyZMPTemplate[9];

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(MITSUBISHI_HEAVY_HDR_MARK);
  IR.space(MITSUBISHI_HEAVY_HDR_SPACE);

  // Data
  for (uint8_t i=0; i<sizeof(MitsubishiHeavyZMPTemplate); i++) {
    Serial.print(F("Byte "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.println(MitsubishiHeavyZMPTemplate[i]);
    IR.sendIRbyte(MitsubishiHeavyZMPTemplate[i], MITSUBISHI_HEAVY_BIT_MARK, MITSUBISHI_HEAVY_ZERO_SPACE, MITSUBISHI_HEAVY_ONE_SPACE);
  }

  // End mark
  IR.mark(MITSUBISHI_HEAVY_BIT_MARK);
  IR.space(0);
}