#include "GreeHeatpumpIR.h"

// This is a protected method, i.e. generic Gree instances cannot be created
GreeHeatpumpIR::GreeHeatpumpIR() : HeatpumpIR()
{
}

GreeGenericHeatpumpIR::GreeGenericHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "gree";
  static const char info[]  PROGMEM = "{\"mdl\":\"gree\",\"dn\":\"Gree\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
  greeModel = GREE_GENERIC;
}

GreeYANHeatpumpIR::GreeYANHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyan";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyan\",\"dn\":\"Gree YAN\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
  greeModel = GREE_YAN;
}

// Support for YAA1FB, FAA1FB1, YB1F2 remotes
GreeYAAHeatpumpIR::GreeYAAHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyaa";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyaa\",\"dn\":\"Gree YAA\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
  greeModel = GREE_YAA;
}

// Support for YAC1FBF remote
GreeYACHeatpumpIR::GreeYACHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyac";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyac\",\"dn\":\"Gree YAC\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
  greeModel = GREE_YAC;
}

void GreeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, false);
}

void GreeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd, false, turboMode);
}

void GreeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode, bool iFeelMode)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = GREE_AIRCON1_POWER_ON;
  uint8_t operatingMode = GREE_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = GREE_AIRCON1_FAN_AUTO;
  uint8_t temperature = 21;
  uint8_t swingV = GREE_VDIR_AUTO;
  uint8_t swingH = GREE_HDIR_AUTO;


  if (powerModeCmd == POWER_OFF)
  {
    powerMode = GREE_AIRCON1_POWER_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = GREE_AIRCON1_MODE_AUTO;
        temperatureCmd = 25;
        break;
      case MODE_HEAT:
        operatingMode = GREE_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = GREE_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = GREE_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_1;
        break;
      case MODE_FAN:
        operatingMode = GREE_AIRCON1_MODE_FAN;
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = GREE_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = GREE_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = GREE_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = GREE_AIRCON1_FAN3;
      break;
  }


  if (greeModel == GREE_YAN)
  {
    switch (swingVCmd)
    {
      case VDIR_AUTO:
      case VDIR_SWING:
        swingV = GREE_VDIR_AUTO;
        break;
      case VDIR_UP:
        swingV = GREE_VDIR_UP;
        break;
      case VDIR_MUP:
        swingV = GREE_VDIR_MUP;
        break;
      case VDIR_MIDDLE:
        swingV = GREE_VDIR_MIDDLE;
        break;
      case VDIR_MDOWN:
        swingV = GREE_VDIR_MDOWN;
        break;
      case VDIR_DOWN:
        swingV = GREE_VDIR_DOWN;
        break;
    }
  }

  if (greeModel == GREE_YAA || greeModel == GREE_YAC)
  {
    switch (swingVCmd)
    {
      case VDIR_AUTO:
        swingV = GREE_VDIR_AUTO;
        break;
      case VDIR_SWING:
        swingV = GREE_VDIR_SWING;
        break;
      case VDIR_UP:
        swingV = GREE_VDIR_UP;
        break;
      case VDIR_MUP:
        swingV = GREE_VDIR_MUP;
        break;
      case VDIR_MIDDLE:
        swingV = GREE_VDIR_MIDDLE;
        break;
      case VDIR_MDOWN:
        swingV = GREE_VDIR_MDOWN;
        break;
      case VDIR_DOWN:
        swingV = GREE_VDIR_DOWN;
        break;
    }

    if (greeModel == GREE_YAC)
    {
      switch (swingHCmd)
      {
        case HDIR_AUTO:
        case HDIR_SWING:
          swingH = GREE_HDIR_SWING;
          break;
        case HDIR_LEFT:
          swingH = GREE_HDIR_LEFT;
          break;
        case HDIR_MLEFT:
          swingH = GREE_HDIR_MLEFT;
          break;
        case HDIR_MIDDLE:
          swingH = GREE_HDIR_MIDDLE;
          break;
        case HDIR_MRIGHT:
          swingH = GREE_HDIR_MRIGHT;
          break;
        case HDIR_RIGHT:
          swingH = GREE_HDIR_RIGHT;
          break;
      }
    }
  }


  if (temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd - 16;
  }

  sendGree(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH, turboMode, iFeelMode);
}

// Send the Gree code
void GreeHeatpumpIR::sendGree(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, bool turboMode, bool iFeelMode)
{
  (void)swingH;

  uint8_t GreeTemplate[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00 };
  //                            0     1     2     3     4     5     6     7

  uint8_t i;

  // Set the Fan speed, operating mode and power state
  GreeTemplate[0] = fanSpeed | operatingMode | powerMode;
  // Set the temperature
  GreeTemplate[1] = temperature;

  // Gree YAN-specific
  if (greeModel == GREE_YAN)
  {
    GreeTemplate[2] = turboMode ? 0x70 : 0x60;
    GreeTemplate[3] = 0x50;
    GreeTemplate[4] = swingV;
  }
  if (greeModel == GREE_YAC)
  {
    GreeTemplate[4] |= (swingH << 4);
    if (iFeelMode)
    {
      GreeTemplate[5] |= (1 << 3);
    }
  }
  if (greeModel == GREE_YAA || greeModel == GREE_YAC)
  {
//    GreeTemplate[2] = 0xE0; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
    GreeTemplate[2] = 0x20; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
    GreeTemplate[3] = 0x50; // bits 4..7 always 0101
    GreeTemplate[6] = 0x20; // YAA1FB, FAA1FB1, YB1F2 bits 4..7 always 0010

    if (turboMode)
    {
      GreeTemplate[2] |= (1 << 4); // Set bit 4 (TURBO)
    }
    if (swingV == GREE_VDIR_SWING)
    {
      GreeTemplate[0] |= (1 << 6); // Enable swing by setting bit 6
    }
    else if (swingV != GREE_VDIR_AUTO)
    {
      GreeTemplate[5] = swingV;
    }
  }

  // Calculate the checksum
  if (greeModel == GREE_YAN)
  {
    GreeTemplate[7] = (
      (GreeTemplate[0] << 4) +
      (GreeTemplate[1] << 4) +
      0xC0);
  }
  else
  {
    GreeTemplate[7] = (((
     (GreeTemplate[0] & 0x0F) +
     (GreeTemplate[1] & 0x0F) +
     (GreeTemplate[2] & 0x0F) +
     (GreeTemplate[3] & 0x0F) +
     ((GreeTemplate[5] & 0xF0) >> 4) +
     ((GreeTemplate[6] & 0xF0) >> 4) +
     ((GreeTemplate[7] & 0xF0) >> 4) +
      0x0A) & 0x0F) << 4) | (GreeTemplate[7] & 0x0F);
  }

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(GREE_AIRCON1_HDR_MARK);
  IR.space(GREE_AIRCON1_HDR_SPACE);

  // Payload part #1
  for (i=0; i<4; i++) {
    IR.sendIRbyte(GreeTemplate[i], GREE_AIRCON1_BIT_MARK, GREE_AIRCON1_ZERO_SPACE, GREE_AIRCON1_ONE_SPACE);
  }
  // Only three first bits of byte 4 are sent, this is always '010'
  IR.mark(GREE_AIRCON1_BIT_MARK);
  IR.space(GREE_AIRCON1_ZERO_SPACE);
  IR.mark(GREE_AIRCON1_BIT_MARK);
  IR.space(GREE_AIRCON1_ONE_SPACE);
  IR.mark(GREE_AIRCON1_BIT_MARK);
  IR.space(GREE_AIRCON1_ZERO_SPACE);

  // Message space
  IR.mark(GREE_AIRCON1_BIT_MARK);
  IR.space(GREE_AIRCON1_MSG_SPACE);

  // Payload part #2
  for (i=4; i<8; i++) {
    IR.sendIRbyte(GreeTemplate[i], GREE_AIRCON1_BIT_MARK, GREE_AIRCON1_ZERO_SPACE, GREE_AIRCON1_ONE_SPACE);
	}

  // End mark
  IR.mark(GREE_AIRCON1_BIT_MARK);
  IR.space(0);
}

// Sends current sensed temperatures, YAC remotes/supporting units only
void GreeYACHeatpumpIR::send(IRSender& IR, uint8_t currentTemperature)
{
  uint8_t GreeTemplate[] = { 0x00, 0x00 };

  GreeTemplate[0] = currentTemperature;
  GreeTemplate[1] = 0xA5;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(GREE_YAC_HDR_MARK);
  IR.space(GREE_YAC_HDR_SPACE);

  // send payload
  IR.sendIRbyte(GreeTemplate[0], GREE_YAC_BIT_MARK, GREE_AIRCON1_ZERO_SPACE, GREE_AIRCON1_ONE_SPACE);
  IR.sendIRbyte(GreeTemplate[1], GREE_YAC_BIT_MARK, GREE_AIRCON1_ZERO_SPACE, GREE_AIRCON1_ONE_SPACE);

  // End mark
  IR.mark(GREE_YAC_BIT_MARK);
  IR.space(0);
}
