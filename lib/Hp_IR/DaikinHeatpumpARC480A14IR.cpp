#include <DaikinHeatpumpARC480A14IR.h>

DaikinHeatpumpARC480A14IR::DaikinHeatpumpARC480A14IR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "daikin_arc480";
  static const char info[]  PROGMEM = "{\"mdl\":\"daikin_arc480\",\"dn\":\"Daikin\",\"mT\":18,\"xT\":30,\"fs\":7,\"maint\":[10,11,12,13,14,15,16,17]}}}";

  _model = model;
  _info = info;
}


void DaikinHeatpumpARC480A14IR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  send(IR, powerModeCmd, operatingModeCmd, fanSpeedCmd, temperatureCmd, swingVCmd, swingHCmd,
       DAIKIN_AIRCON_COMFORT_OFF, DAIKIN_AIRCON_ECONO_OFF, DAIKIN_AIRCON_SENSOR_OFF, DAIKIN_AIRCON_QUIET_OFF, DAIKIN_AIRCON_POWERFUL_OFF);
}

// Daikin numeric values to command bytes
void DaikinHeatpumpARC480A14IR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, uint8_t comfortModeCmd, uint8_t econoCmd, uint8_t sensorCmd, uint8_t quietCmd, uint8_t powerfulCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = DAIKIN_AIRCON_MODE_OFF;
  uint8_t fanSpeed      = DAIKIN_AIRCON_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = DAIKIN_AIRCON_SWING_OFF;
  uint8_t comfortMode   = DAIKIN_AIRCON_COMFORT_OFF;
  uint8_t econo         = DAIKIN_AIRCON_ECONO_OFF;
  uint8_t sensor        = DAIKIN_AIRCON_SENSOR_OFF;
  uint8_t quiet         = DAIKIN_AIRCON_QUIET_OFF;
  uint8_t powerful      = DAIKIN_AIRCON_POWERFUL_OFF;

  switch (powerModeCmd)
  {
    case POWER_ON:
      operatingMode |= DAIKIN_AIRCON_MODE_ON;
      break;
    case POWER_OFF:
      operatingMode |= DAIKIN_AIRCON_MODE_OFF;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode |= DAIKIN_AIRCON_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode |= DAIKIN_AIRCON_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode |= DAIKIN_AIRCON_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode |= DAIKIN_AIRCON_MODE_DRY;
      temperatureCmd = 0x24;
      break;
    case MODE_FAN:
      operatingMode |= DAIKIN_AIRCON_MODE_FAN;
      temperatureCmd = 0xC0;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = DAIKIN_AIRCON_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = DAIKIN_AIRCON_FAN1;
      break;
    case FAN_2:
      fanSpeed = DAIKIN_AIRCON_FAN2;
      break;
    case FAN_3:
      fanSpeed = DAIKIN_AIRCON_FAN3;
      break;
    case FAN_4:
      fanSpeed = DAIKIN_AIRCON_FAN4;
      break;
    case FAN_5:
      fanSpeed = DAIKIN_AIRCON_FAN5;
      break;
    case FAN_SILENT:
      fanSpeed = DAIKIN_AIRCON_FAN_SILENT;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = DAIKIN_AIRCON_SWING_ON;
      break;
    case VDIR_UP:
      swingV = DAIKIN_AIRCON_SWING_OFF;
      break;
  }

  if ((operatingModeCmd == MODE_HEAT && temperatureCmd >= 10 && temperatureCmd <= 30) ||
      (temperatureCmd >= 18 && temperatureCmd <= 30))
  {
    temperature = temperatureCmd << 1;
  }

  if (comfortModeCmd) {
    comfortMode = DAIKIN_AIRCON_COMFORT_ON;
    powerful = DAIKIN_AIRCON_POWERFUL_OFF;
    fanSpeed  = DAIKIN_AIRCON_FAN_AUTO;
    swingV    = DAIKIN_AIRCON_SWING_OFF;
  }
  else {
    comfortMode = DAIKIN_AIRCON_COMFORT_OFF;
  }

  if (econoCmd) {
    econo     = DAIKIN_AIRCON_ECONO_ON;
    powerful  = DAIKIN_AIRCON_POWERFUL_OFF;
  }
  else {
    econo     = DAIKIN_AIRCON_ECONO_OFF;
  }

  if (sensorCmd) {
    sensor    = DAIKIN_AIRCON_SENSOR_ON;
  }
  else {
    sensor     = DAIKIN_AIRCON_SENSOR_OFF;
  }

  if (quietCmd) {
    quiet     = DAIKIN_AIRCON_QUIET_ON;
    powerful  = DAIKIN_AIRCON_POWERFUL_OFF;
  }
  else {
    quiet     = DAIKIN_AIRCON_QUIET_OFF;
  }
  if (powerfulCmd) {
    powerful  = DAIKIN_AIRCON_POWERFUL_ON;
    quiet     = DAIKIN_AIRCON_QUIET_OFF;
    econo     = DAIKIN_AIRCON_SENSOR_OFF;
    comfortMode = DAIKIN_AIRCON_COMFORT_OFF;
  }
  else {
    powerful  = DAIKIN_AIRCON_POWERFUL_OFF;
  }

  sendDaikin(IR, operatingMode, fanSpeed, temperature, swingV, swingHCmd, comfortMode, econo, sensor, quiet, powerful);
}

// Send the Daikin code
void DaikinHeatpumpARC480A14IR::sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t comfortMode, uint8_t econo, uint8_t sensor, uint8_t quiet, uint8_t powerful)
{
  (void)swingH;

  uint8_t daikinTemplate[19] = {
  // cold @ 18c fan speed 5       MODE  TEMP         FAN                     \QUIET POWERFUL/     \specials/
  //0x11  0xDA  0x27  0x00  0x00  0x31  0x24  0x00  0xA0  0x00  0x00  0x00  0x00  0x00  0x00  0xC5  0x02  0x08  0xD6
    0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xC5, 0x00, 0x08, 0x00 };
   // 0     1     2     3     4     5     6     7     8     9    10     11    12    13    14    15    16    17    18
   //                                                           | on/off timers  |
  daikinTemplate[5] = operatingMode;
  daikinTemplate[6] = temperature;
  daikinTemplate[8] = fanSpeed + swingV;
  daikinTemplate[13] = quiet + powerful;
  daikinTemplate[16] = comfortMode + econo + sensor;

  // Checksum calculation
  // * Checksums at bytes 7 and 15 are calculated the same way
  uint8_t checksum = 0x00;

  for (int i=0; i<18; i++) {
    checksum += daikinTemplate[i];
  }

  daikinTemplate[18] = checksum;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(DAIKIN_AIRCON_HDR_MARK);
  IR.space(DAIKIN_AIRCON_HDR_SPACE);

  // First header
  for (int i=0; i<19; i++) {
    IR.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(0);
}
