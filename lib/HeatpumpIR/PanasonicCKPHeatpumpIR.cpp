#include <PanasonicCKPHeatpumpIR.h>


PanasonicCKPHeatpumpIR::PanasonicCKPHeatpumpIR()
{
  static const char PROGMEM model[] PROGMEM = "panasonic_ckp";
  static const char PROGMEM info[]  PROGMEM = "{\"mdl\":\"panasonic_ckp\",\"dn\":\"Panasonic CKP\",\"mT\":16,\"xT\":30,\"fs\":6}";

  _model = model;
  _info = info;
}


// Panasonic CKP numeric values to command uint8_ts
void PanasonicCKPHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = false;
  uint8_t operatingMode = PANASONIC_AIRCON1_MODE_KEEP;
  uint8_t fanSpeed      = PANASONIC_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = PANASONIC_AIRCON1_VS_UP;
  uint8_t swingH        = PANASONIC_AIRCON1_HS_SWING;

  switch (powerModeCmd)
  {
    case POWER_ON:
      powerMode = true;
      break;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode |= PANASONIC_AIRCON1_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode |= PANASONIC_AIRCON1_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode |= PANASONIC_AIRCON1_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode |= PANASONIC_AIRCON1_MODE_DRY;
      break;
    case MODE_FAN:
      operatingMode |= PANASONIC_AIRCON1_MODE_FAN;
      temperatureCmd = 27; // Temperature is always 27 in FAN mode
      break;
    default:
      operatingMode |= PANASONIC_AIRCON1_MODE_HEAT;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = PANASONIC_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = PANASONIC_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = PANASONIC_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = PANASONIC_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = PANASONIC_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = PANASONIC_AIRCON1_FAN5;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_SWING:
      swingV = PANASONIC_AIRCON1_VS_SWING;
      break;
    case VDIR_UP:
      swingV = PANASONIC_AIRCON1_VS_UP;
      break;
    case VDIR_MUP:
      swingV = PANASONIC_AIRCON1_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = PANASONIC_AIRCON1_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = PANASONIC_AIRCON1_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = PANASONIC_AIRCON1_VS_DOWN;
      break;
  }

  switch (swingHCmd)
  {
    case HDIR_SWING:
      swingH = PANASONIC_AIRCON1_HS_SWING;
      break;
    case HDIR_AUTO: // Well, just set it to manual
      swingH = PANASONIC_AIRCON1_HS_MANUAL;
      break;
  }

  sendPanasonicCKP(IR, operatingMode, fanSpeed, temperature, swingV, swingH);
  delay(1000); // Sleep 1 second between the messages

  // This will change the power state in one minute from now
  sendPanasonicCKPOnOffTimerCancel(IR, powerMode, false);
/*
  // Send the 'timer cancel' signal 2 minutes later
  if (panasonicCancelTimer != 0)
  {
    timer.stop(panasonicCancelTimer);
    panasonicCancelTimer = 0;
  }

  // Note that the argument to 'timer.after' has to be explicitly cast into 'long'
  panasonicCancelTimer = timer.after(2L*60L*1000L, sendPanasonicCKPCancelTimer);

  Serial.print(F("'Timer cancel' timer ID: "));
  Serial.println(panasonicCancelTimer);
*/
}

// Send the Panasonic CKP code
void PanasonicCKPHeatpumpIR::sendPanasonicCKP(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t sendBuffer[4];

  // Fan speed & temperature, temperature needs to be 27 in FAN mode
  if (operatingMode == PANASONIC_AIRCON1_MODE_FAN || operatingMode == (PANASONIC_AIRCON1_MODE_FAN | PANASONIC_AIRCON1_MODE_KEEP ))
  {
    temperature = 27;
  }

  sendBuffer[0] = fanSpeed | (temperature - 15);

  // Power toggle & operation mode
  sendBuffer[1] = operatingMode;

  // Swings
  sendBuffer[2] = swingV | swingH;

  // Always 0x36
  sendBuffer[3]  = 0x36;

  // Send the code
  sendPanasonicCKPraw(IR, sendBuffer);
}

// Send the Panasonic CKP raw code
void PanasonicCKPHeatpumpIR::sendPanasonicCKPraw(IRSender& IR, uint8_t sendBuffer[])
{
  // 40 kHz PWM frequency
  IR.setFrequency(38);

  // Header, two first uint8_ts repeated
  IR.mark(PANASONIC_AIRCON1_HDR_MARK);
  IR.space(PANASONIC_AIRCON1_HDR_SPACE);

  for (int i=0; i<2; i++) {
    IR.sendIRbyte(sendBuffer[0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    IR.mark(PANASONIC_AIRCON1_HDR_MARK);
    IR.space(PANASONIC_AIRCON1_HDR_SPACE);
  }

  // Pause

  IR.mark(PANASONIC_AIRCON1_BIT_MARK);
  IR.space(PANASONIC_AIRCON1_MSG_SPACE);

  // Header, two last uint8_ts repeated

  IR.mark(PANASONIC_AIRCON1_HDR_MARK);
  IR.space(PANASONIC_AIRCON1_HDR_SPACE);

  for (int i=0; i<2; i++) {
    IR.sendIRbyte(sendBuffer[2], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[2], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[3], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[3], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    IR.mark(PANASONIC_AIRCON1_HDR_MARK);
    IR.space(PANASONIC_AIRCON1_HDR_SPACE);
  }

  IR.mark(PANASONIC_AIRCON1_BIT_MARK);
  IR.space(0);
}

// Send the Panasonic CKP On/Off code
//
// CKP does not have discrete ON/OFF commands, but this can be emulated by using the timer
// The side-effects of using the timer are:
// * ONE minute delay before the power state changes
// * the 'TIMER' led (orange) is lit
// * a timer event is scheduled to cancel the timer after TWO minutes (the 'TIMER' led turns off
void PanasonicCKPHeatpumpIR::sendPanasonicCKPOnOffTimerCancel(IRSender& IR, boolean powerState, boolean cancelTimer)
{
  static const uint8_t ON_msg[] PROGMEM =     { 0x7F, 0x38, 0xBF, 0x38, 0x10, 0x3D, 0x80, 0x3D, 0x09, 0x34, 0x80, 0x34 }; //  ON at 00:10, time now 00:09, no OFF timing
  static const uint8_t OFF_msg[] PROGMEM =    { 0x10, 0x38, 0x80, 0x38, 0x7F, 0x3D, 0xBF, 0x3D, 0x09, 0x34, 0x80, 0x34 }; // OFF at 00:10, time now 00:09, no ON timing
  static const uint8_t CANCEL_msg[] PROGMEM = { 0x7F, 0x38, 0xBF, 0x38, 0x7F, 0x3D, 0xBF, 0x3D, 0x17, 0x34, 0x80, 0x34 }; // Timer CANCEL

  // Save some SRAM by only having one copy of the template on the SRAM
  uint8_t sendBuffer[sizeof(ON_msg)];

  if ( cancelTimer == true ) {
    memcpy_P(sendBuffer, CANCEL_msg, sizeof(ON_msg));
  } else if ( powerState == true ) {
    memcpy_P(sendBuffer, ON_msg, sizeof(ON_msg));
  } else {
    memcpy_P(sendBuffer, OFF_msg, sizeof(ON_msg));
  }

  // 40 kHz PWM frequency
  IR.setFrequency(38);

  for (int i=0; i<6; i++) {
    IR.mark(PANASONIC_AIRCON1_HDR_MARK);
    IR.space(PANASONIC_AIRCON1_HDR_SPACE);

    IR.sendIRbyte(sendBuffer[i*2 + 0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[i*2 + 0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[i*2 + 1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(sendBuffer[i*2 + 1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    IR.mark(PANASONIC_AIRCON1_HDR_MARK);
    IR.space(PANASONIC_AIRCON1_HDR_SPACE);

    if ( i < 5 ) {
      IR.mark(PANASONIC_AIRCON1_BIT_MARK);
      IR.space(PANASONIC_AIRCON1_MSG_SPACE);
    }
  }

  IR.mark(PANASONIC_AIRCON1_BIT_MARK);
  IR.space(0);
}

// Send the Panasonic CKP timer cancel
void PanasonicCKPHeatpumpIR::sendPanasonicCKPCancelTimer(IRSender& IR)
{
  Serial.println(F("Sending Panasonic CKP timer cancel"));

  sendPanasonicCKPOnOffTimerCancel(IR, false, true);
}

