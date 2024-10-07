#include "GreeHeatpumpIR.h"

namespace {

void convert_params(
  uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd,
  uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode, bool iFeelMode,
  uint8_t & powerMode, uint8_t & operatingMode, uint8_t & fanSpeed,
  uint8_t & temperature, uint8_t & swingV, uint8_t & swingH)
{
  // Sensible defaults for the heat pump mode
  powerMode = GREE_AIRCON1_POWER_ON;
  operatingMode = GREE_AIRCON1_MODE_HEAT;
  fanSpeed = GREE_AIRCON1_FAN_AUTO;
  temperature = 21;
  swingV = GREE_VDIR_AUTO;
  swingH = GREE_HDIR_AUTO;

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

  switch (swingHCmd)
  {
    case HDIR_AUTO:
      swingH = GREE_HDIR_AUTO;
      break;
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

  if (temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd - 16;
  }
}

}

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
}

GreeYANHeatpumpIR::GreeYANHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyan";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyan\",\"dn\":\"Gree YAN\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

// Support for YAA1FB, FAA1FB1, YB1F2 remotes
GreeYAAHeatpumpIR::GreeYAAHeatpumpIR() : GreeHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyaa";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyaa\",\"dn\":\"Gree YAA\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

// Support for YAC1FBF remote
GreeYACHeatpumpIR::GreeYACHeatpumpIR() : GreeiFeelHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyac";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyac\",\"dn\":\"Gree YAC\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

// Support for YT1F remote
GreeYTHeatpumpIR::GreeYTHeatpumpIR() : GreeiFeelHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyt";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyt\",\"dn\":\"Gree YT\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

// Support for YAP1F remote
GreeYAPHeatpumpIR::GreeYAPHeatpumpIR() : GreeiFeelHeatpumpIR()
{
  static const char model[] PROGMEM = "greeyap";
  static const char info[]  PROGMEM = "{\"mdl\":\"greeyap\",\"dn\":\"Gree YAP\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
}

const GreeHeatpumpIR::Timings & GreeHeatpumpIR::getTimings() const {
    static Timings timings = {
        9000,
        4000,
        620,
        1600,
        540,
        19000,
        8200,
        3800,
        650,
    };
    return timings;
};

void GreeHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode, bool iFeelMode)
{
  uint8_t powerMode, operatingMode, fanSpeed, temperature, swingV, swingH;

  convert_params(
    powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd,
    swingVCmd, swingHCmd, turboMode, iFeelMode,
    powerMode, operatingMode, fanSpeed,
    temperature, swingV, swingH);

  sendGree(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH, turboMode, iFeelMode);
}

// Send the Gree code
void GreeHeatpumpIR::sendGree(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, bool turboMode, bool iFeelMode)
{
  uint8_t buffer[9];

  generateCommand(
      buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  calculateChecksum(buffer);
  sendBuffer(IR, buffer);
}

void GreeHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {

  memset(buffer, 0, 8);

  // Set the Fan speed, operating mode and power state
  buffer[0] = fanSpeed | operatingMode | powerMode;

  // Set the temperature
  buffer[1] = temperature;
}

void GreeYANHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  GreeHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  buffer[2] = turboMode ? 0x70 : 0x60;
  buffer[3] = 0x50;
  if (swingV == GREE_VDIR_SWING)
    swingV = GREE_VDIR_AUTO;
  buffer[4] = swingV;
  buffer[5] |= 0x20;
}

void GreeYAAHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  GreeHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  buffer[2] = GREE_LIGHT_BIT; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
  buffer[3] = 0x50; // bits 4..7 always 0101
  buffer[5] |= 0x20;
  buffer[6] = 0x20; // YAA1FB, FAA1FB1, YB1F2 bits 4..7 always 0010

  if (turboMode)
  {
    buffer[2] |= GREE_TURBO_BIT;
  }
  if (swingV == GREE_VDIR_SWING)
  {
    buffer[0] |= GREE_VSWING; // Enable swing by setting bit 6
  }
  else if (swingV != GREE_VDIR_AUTO)
  {
    buffer[5] = swingV;
  }
}

void GreeiFeelHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  GreeHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  if (iFeelMode) {
    buffer[5] |= GREE_IFEEL_BIT;
  }
}

void GreeYACHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  GreeiFeelHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  if (swingH == GREE_HDIR_AUTO)
    swingH = GREE_HDIR_SWING;

  buffer[4] |= (swingH << 4); // GREE_YT will ignore packets where this is set

  buffer[2] = GREE_LIGHT_BIT; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
  buffer[3] = 0x50; // bits 4..7 always 0101
  buffer[5] |= 0x20;
  buffer[6] = 0x20; // YAA1FB, FAA1FB1, YB1F2 bits 4..7 always 0010

  if (turboMode)
  {
    buffer[2] |= GREE_TURBO_BIT;
  }
  if (swingV == GREE_VDIR_SWING)
  {
    buffer[0] |= GREE_VSWING; // Enable swing by setting bit 6
  }
  else if (swingV != GREE_VDIR_AUTO)
  {
    buffer[5] = swingV;
  }
}

void GreeYTHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  GreeiFeelHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  buffer[2] = GREE_LIGHT_BIT | GREE_HEALTH_BIT; // HEALTH is always on for GREE_YT
  buffer[3] = 0x50; // bits 4..7 always 0101

  if (turboMode)
  {
    buffer[2] |= GREE_TURBO_BIT;
  }
  if (swingV == GREE_VDIR_SWING)
  {
    buffer[0] |= GREE_VSWING; // Enable swing by setting bit 6
    buffer[4] = swingV;
  }
}

void GreeYAPHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) {
  generateCommand(buffer,
                  powerMode, operatingMode,
                  fanSpeed, temperature,
                  swingV, swingH,
                  turboMode, iFeelMode,
                  true);
}

void GreeYAPHeatpumpIR::generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode,
            bool light, bool xfan,
            bool health, bool valve,
            bool sthtMode, bool enableWiFi) {

  GreeiFeelHeatpumpIR::generateCommand(buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode);

  buffer[2] =
    (turboMode ? (1 << 4) : 0) |
    (light ? (1 << 5) : 0) |
    (health ? (1 << 6) : 0) |
    (xfan ? (1 << 7) : 0);

  buffer[3] = 0x50 | (valve ? (1 << 0) : 0);  // bits 4..7 always 0101

  buffer[4] = swingV | (swingH << 4);

  buffer[5] = 0x82 |
    (iFeelMode ? (1 << 2) : 0) |  // note that this is different than in the other devices
    (enableWiFi ? (1 << 6) : 0);

  buffer[7] = (sthtMode ? (1 << 2) : 0);

  memset(buffer + 8, 0, 16);
  memcpy(buffer + 8, buffer, 3);

  buffer[8 + 3] = 0x70 |
    (valve ? (1 << 0) : 0);

  buffer[16 + 3] = 0xA0;
  buffer[16 + 7] = 0xA0;
}

void GreeHeatpumpIR::calculateChecksum(uint8_t * buffer) {
  buffer[8] = (((
   (buffer[0] & 0x0F) +
   (buffer[1] & 0x0F) +
   (buffer[2] & 0x0F) +
   (buffer[3] & 0x0F) +
   ((buffer[5] & 0xF0) >> 4) +
   ((buffer[6] & 0xF0) >> 4) +
   ((buffer[7] & 0xF0) >> 4) +
    0x0A) & 0x0F) << 4) | (buffer[7] & 0x0F);
}

void GreeYANHeatpumpIR::calculateChecksum(uint8_t * buffer) {
  buffer[8] = (
    (buffer[0] << 4) +
    (buffer[1] << 4) +
    0xC0);
}

void GreeHeatpumpIR::sendBuffer(IRSender& IR, const uint8_t * buffer, size_t len) {
  const auto & timings = getTimings();

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  for (size_t pos = 0; pos < len; pos += 8) {
    // All but the first group must be preceded by a space
    if (pos) {
      IR.mark(timings.bit_mark);
      IR.space(timings.msg_space);
    }

    // Send Header mark
    IR.mark(timings.hdr_mark);
    IR.space(timings.hdr_space);

    // Payload part #1
    for (size_t i = 0; i < 4; i++) {
      IR.sendIRbyte(buffer[pos + i], timings.bit_mark, timings.zero_space, timings.one_space);
    }
    // Only three first bits of byte 4 are sent, this is always '010'
    IR.mark(timings.bit_mark);
    IR.space(timings.zero_space);
    IR.mark(timings.bit_mark);
    IR.space(timings.one_space);
    IR.mark(timings.bit_mark);
    IR.space(timings.zero_space);

    // Message space
    IR.mark(timings.bit_mark);
    IR.space(timings.msg_space);

    // Payload part #2
    for (size_t i = 5; i < 9; i++) {
      IR.sendIRbyte(buffer[pos + i], timings.bit_mark, timings.zero_space, timings.one_space);
    }

    // End mark
    IR.mark(timings.bit_mark);
    IR.space(0);
  }
}

const GreeHeatpumpIR::Timings & GreeYAPHeatpumpIR::getTimings() const {
    static Timings timings = {
        9000,
        4500,
        650,
        1643,
        510,
        20000,
        6000,
        3000,
        650,
    };
    return timings;
};

// Send the Gree code
void GreeYAPHeatpumpIR::sendGree(
        IRSender& IR,
        uint8_t powerMode, uint8_t operatingMode,
        uint8_t fanSpeed, uint8_t temperature,
        uint8_t swingV, uint8_t swingH,
        bool turboMode, bool iFeelMode) {
  sendGree(IR,
           powerMode, operatingMode,
           fanSpeed, temperature,
           swingV, swingH,
           turboMode, iFeelMode,
           true);
}

void GreeYAPHeatpumpIR::sendGree(
        IRSender& IR,
        uint8_t powerMode, uint8_t operatingMode,
        uint8_t fanSpeed, uint8_t temperature,
        uint8_t swingV, uint8_t swingH,
        bool turboMode, bool iFeelMode,
        bool light, bool xfan,
        bool health, bool valve,
        bool sthtMode, bool enableWiFi) {

  uint8_t buffer[24];

  generateCommand(
      buffer,
      powerMode, operatingMode,
      fanSpeed, temperature,
      swingV, swingH,
      turboMode, iFeelMode,
      light, xfan,
      health, valve,
      sthtMode, enableWiFi);

  calculateChecksum(buffer);
  calculateChecksum(buffer + 8);

  sendBuffer(IR, buffer, 24);
}

void GreeYAPHeatpumpIR::send(
          IRSender& IR,
          uint8_t powerModeCmd, uint8_t operatingModeCmd,
          uint8_t fanSpeedCmd, uint8_t temperatureCmd,
          uint8_t swingVCmd, uint8_t swingHCmd,
          bool turboMode, bool iFeelMode,
          bool light, bool xfan,
          bool health, bool valve,
          bool sthtMode, bool enableWiFi) {
  uint8_t powerMode, operatingMode, fanSpeed, temperature, swingV, swingH;

  convert_params(
    powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd,
    swingVCmd, swingHCmd, turboMode, iFeelMode,
    powerMode, operatingMode, fanSpeed,
    temperature, swingV, swingH);

  sendGree(IR,
           powerMode, operatingMode, fanSpeed, temperature,
           swingV, swingH, turboMode, iFeelMode,
           light, xfan, health,
           valve, sthtMode, enableWiFi);
}

// Sends current sensed temperatures, YAC remotes/supporting units only
void GreeiFeelHeatpumpIR::send(IRSender& IR, uint8_t currentTemperature)
{
  uint8_t GreeTemplate[] = { 0x00, 0x00 };

  GreeTemplate[0] = currentTemperature;
  GreeTemplate[1] = 0xA5;

  const auto & timings = getTimings();

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(timings.ifeel_hdr_mark);
  IR.space(timings.ifeel_hdr_space);

  // send payload
  IR.sendIRbyte(GreeTemplate[0], timings.ifeel_bit_mark, timings.zero_space, timings.one_space);
  IR.sendIRbyte(GreeTemplate[1], timings.ifeel_bit_mark, timings.zero_space, timings.one_space);

  // End mark
  IR.mark(timings.ifeel_bit_mark);
  IR.space(0);
}
