#include "KY26HeatpumpIR.h"

KY26HeatpumpIR::KY26HeatpumpIR() : HeatpumpIR() {
  static const char model[] PROGMEM = "KY-26";
  static const char info[] PROGMEM =
      "{\"mdl\":\"ky26\",\"dn\":\"KY-26\",\"mT\":15,\"xT\":31,\"fs\":3}";

  _model = model;
  _info = info;
}

void KY26HeatpumpIR::send(IRSender &IR, uint8_t powerModeCmd,
                          uint8_t operatingModeCmd, uint8_t fanSpeedCmd,
                          uint8_t temperatureCmd, uint8_t swingVCmd,
                          uint8_t swingHCmd, uint8_t timerHourCmd,
                          bool timerHalfHourCmd) {
  uint8_t powerMode = KY26_POWER_OFF;
  uint8_t operatingMode = KY26_MODE_AUTO;
  uint8_t fanSpeed = KY26_FAN1;
  uint8_t temperature = 24;
  uint8_t timer = KY26_TIMER_OFF;

  // Operating mode
  switch (operatingModeCmd) {
  case MODE_AUTO:
    operatingMode = KY26_MODE_AUTO;
    break;
  case MODE_COOL:
    operatingMode = KY26_MODE_COOL;
    break;
  case MODE_DRY:
    operatingMode = KY26_MODE_DRY;
    break;
  case MODE_FAN:
    operatingMode = KY26_MODE_FAN;
    break;
  }

  // Fan speed
  switch (fanSpeedCmd) {
  case FAN_1:
    fanSpeed = KY26_FAN1;
    break;
  case FAN_2:
    fanSpeed = KY26_FAN2;
    break;
  case FAN_3:
    fanSpeed = KY26_FAN3;
    break;
  }

  // Temperature
  if (temperatureCmd >= 15 && temperatureCmd <= 31) {
    temperature = temperatureCmd == 15 ? 0 : temperatureCmd;
  }

  // Timer
  if (timerHourCmd > 0 && timerHourCmd <= 12) {
    timer = timerHourCmd | KY26_TIMER_ON;

    if (timerHalfHourCmd) {
      timer |= KY26_TIMER_HALF_HOUR;
    }
  }

  // Power mode
  // This heatpump does not have a power on command, but we can simulate it by
  // sending a power off command followed by a power toggle command.
  if (powerModeCmd == POWER_ON) {
    sendKY26(IR, powerMode, operatingMode, fanSpeed, temperature, timer);
    powerMode = KY26_POWER_ONOFF;
  }

  return sendKY26(IR, powerMode, operatingMode, fanSpeed, temperature, timer);
}

void KY26HeatpumpIR::sendKY26(IRSender &IR, uint8_t powerModeCmd,
                              uint8_t operatingModeCmd, uint8_t fanSpeedCmd,
                              uint8_t temperatureCmd, uint8_t timerCmd) {
  uint8_t KY26Template[] = {0x00, 0x00, 0x00, 0x00};

  KY26Template[0] |= powerModeCmd | operatingModeCmd | fanSpeedCmd;
  KY26Template[1] |= timerCmd;
  KY26Template[2] |= temperatureCmd;

  KY26Template[3] = (KY26Template[0] + KY26Template[1] + KY26Template[2]) % 256;

  IR.setFrequency(38);

  // Header
  IR.mark(KY26_HDR_MARK);
  IR.space(KY26_HDR_SPACE);

  for (unsigned int i = 0; i < sizeof(KY26Template); i++) {
    IR.sendIRbyte(KY26Template[i], KY26_BIT_MARK, KY26_ZERO_SPACE,
                  KY26_ONE_SPACE);
  }

  // Footer
  IR.mark(KY26_BIT_MARK);
  IR.space(0);
}
