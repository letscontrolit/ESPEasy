#include <MitsubishiHeatpumpIR.h>

#ifdef USE_TIME_H
#include <time.h>
#endif

#ifdef IR_SEND_TIME
int sendHour = 16;
int sendMinute = 10;
int sendWeekday = 3;
#endif
#ifdef IR_DEBUG_PACKET
char IRPacket[180];
#endif


// These are protected methods, i.e. generic Mitsubishi instances cannot be created directly
MitsubishiHeatpumpIR::MitsubishiHeatpumpIR() : HeatpumpIR()
{
}


// The different models just set the model accordingly

MitsubishiFDHeatpumpIR::MitsubishiFDHeatpumpIR() : MitsubishiHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_fd";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_fd\",\"dn\":\"Mitsubishi FD\",\"mT\":16,\"xT\":31,\"fs\":5}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHI_FD;
}

MitsubishiFEHeatpumpIR::MitsubishiFEHeatpumpIR() : MitsubishiHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_fe";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_fe\",\"dn\":\"Mitsubishi FE\",\"mT\":16,\"xT\":31,\"fs\":5,\"maint\":[10]}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHI_FE;
}

MitsubishiMSYHeatpumpIR::MitsubishiMSYHeatpumpIR() : MitsubishiHeatpumpIR()
{
  static const char model[] PROGMEM = "mitsubishi_msy";
  static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_msy\",\"dn\":\"Mitsubishi MSY\",\"mT\":16,\"xT\":31,\"fs\":5,\"maint\":[10]}";

  _model = model;
  _info = info;

  _mitsubishiModel = MITSUBISHI_MSY;
}

MitsubishiFAHeatpumpIR::MitsubishiFAHeatpumpIR() : MitsubishiHeatpumpIR()
{
	static const char model[] PROGMEM = "mitsubishi_fa";
	static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_fa\",\"dn\":\"Mitsubishi FA\",\"mT\":16,\"xT\":31,\"fs\":5}"; //TODO: What should be here?

	_model = model;
	_info = info;

	_mitsubishiModel = MITSUBISHI_FA;
}

MitsubishiKJHeatpumpIR::MitsubishiKJHeatpumpIR() : MitsubishiHeatpumpIR()
{
	static const char model[] PROGMEM = "mitsubishi_kj";
	static const char info[]  PROGMEM = "{\"mdl\":\"mitsubishi_kj\",\"dn\":\"Mitsubishi KJ\",\"mT\":16,\"xT\":31,\"fs\":5,\"maint\":[10]}";

	_model = model;
	_info = info;

	_mitsubishiModel = MITSUBISHI_KJ;
}

void MitsubishiHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t powerMode     = MITSUBISHI_AIRCON1_MODE_ON;
  uint8_t operatingMode = MITSUBISHI_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed      = MITSUBISHI_AIRCON1_FAN_AUTO;
  uint8_t temperature   = 23;
  uint8_t swingV        = MITSUBISHI_AIRCON1_VS_AUTO;
  uint8_t swingH        = MITSUBISHI_AIRCON1_HS_SWING;

  if (powerModeCmd == 0)
  {
    powerMode = MITSUBISHI_AIRCON1_MODE_OFF;
  }

  if (_mitsubishiModel == MITSUBISHI_FA) // set operating model for FA
  {
	  switch (operatingModeCmd)
	  {
      case MODE_AUTO:
        operatingMode = MITSUBISHI_AIRCON3_MODE_AUTO;
        break;
      case MODE_HEAT:
        operatingMode = MITSUBISHI_AIRCON3_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = MITSUBISHI_AIRCON3_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = MITSUBISHI_AIRCON3_MODE_DRY;
        break;
	  }
  }
  else if (_mitsubishiModel == MITSUBISHI_MSY)
  {
	  operatingMode = MITSUBISHI_AIRCON2_MODE_COOL;
	  switch (operatingModeCmd)
	  {
      case MODE_AUTO:
        operatingMode = MITSUBISHI_AIRCON2_MODE_IFEEL;
        break;
      case MODE_HEAT:
        operatingMode = MITSUBISHI_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = MITSUBISHI_AIRCON2_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = MITSUBISHI_AIRCON1_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = MITSUBISHI_AIRCON2_MODE_FAN;
        break;
	  }
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = MITSUBISHI_AIRCON1_MODE_AUTO;
        break;
      case MODE_HEAT:
        operatingMode = MITSUBISHI_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = MITSUBISHI_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = MITSUBISHI_AIRCON1_MODE_DRY;
        break;
      case MODE_FAN:
        if (_mitsubishiModel == MITSUBISHI_FE) {
          operatingMode = MITSUBISHI_AIRCON1_MODE_FAN;
          temperatureCmd = 24;
        } else {
          operatingMode = MITSUBISHI_AIRCON1_MODE_COOL;
          // Temperature needs to be set to 31 degrees for 'simulated' FAN mode
          temperatureCmd = 31;
        }
        break;
      case MODE_MAINT: // Maintenance mode is just the heat mode at +10, FAN5
        if (_mitsubishiModel == MITSUBISHI_FE || _mitsubishiModel == MITSUBISHI_KJ) {
          operatingMode |= MITSUBISHI_AIRCON1_MODE_HEAT;
          temperature = 10; // Default to +10 degrees
          fanSpeedCmd = FAN_AUTO;
        }
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = MITSUBISHI_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = MITSUBISHI_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = MITSUBISHI_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = MITSUBISHI_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = MITSUBISHI_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = MITSUBISHI_AIRCON1_FAN5; // Quiet mode on KJ
      break;

  }

  if ( temperature != 10 && temperatureCmd > 16 && temperatureCmd < 32)
  {
    temperature = temperatureCmd;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
      swingV = MITSUBISHI_AIRCON1_VS_AUTO;
      break;
    case VDIR_SWING:
      swingV = MITSUBISHI_AIRCON1_VS_SWING;
      break;
    case VDIR_UP:
      swingV = MITSUBISHI_AIRCON1_VS_UP;
      break;
    case VDIR_MUP:
      swingV = MITSUBISHI_AIRCON1_VS_MUP;
      break;
    case VDIR_MIDDLE:
      swingV = MITSUBISHI_AIRCON1_VS_MIDDLE;
      break;
    case VDIR_MDOWN:
      swingV = MITSUBISHI_AIRCON1_VS_MDOWN;
      break;
    case VDIR_DOWN:
      swingV = MITSUBISHI_AIRCON1_VS_DOWN;
      break;
  }

  // KJ has no horizontal swing. Instead 0 means 2flow, else 1flow operation
  if (_mitsubishiModel ==  MITSUBISHI_KJ) {
    swingH = swingHCmd; // We pass the value unmodified
  }
  else {
    switch (swingHCmd)
    {
      case HDIR_AUTO:
      case HDIR_SWING:
        swingH = MITSUBISHI_AIRCON1_HS_SWING;
        break;
      case HDIR_MIDDLE:
        swingH = MITSUBISHI_AIRCON1_HS_MIDDLE;
        break;
      case HDIR_LEFT:
        swingH = MITSUBISHI_AIRCON1_HS_LEFT;
        break;
      case HDIR_MLEFT:
        swingH = MITSUBISHI_AIRCON1_HS_MLEFT;
        break;
      case HDIR_RIGHT:
        swingH = MITSUBISHI_AIRCON1_HS_RIGHT;
        break;
      case HDIR_MRIGHT:
        swingH = MITSUBISHI_AIRCON1_HS_MRIGHT;
        break;
    }
  }

  sendMitsubishi(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

void MitsubishiHeatpumpIR::sendMitsubishi(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
  uint8_t MitsubishiTemplate[] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x48, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00 };
  //                                  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17
  uint8_t TempTranslate[]      = { 0x82, 0x00, 0x01, 0x02, 0x03, 0x80, 0x81 };
  //                                Sun   Mon   Tue   Wed   Thu   Fri   Sat

  uint8_t checksum = 0x00;
#ifdef USE_TIME_H
  time_t now;
  struct tm * timeinfo;
#endif
  char pbyte[16];

  // MSY has a bit different template
  if (_mitsubishiModel == MITSUBISHI_MSY) {
    MitsubishiTemplate[14] = 0x00;
    MitsubishiTemplate[15] = 0x00;
  }

  // FA also has a bit different template
  if (_mitsubishiModel == MITSUBISHI_FA)
  {
    MitsubishiTemplate[10] = 0x00;
    MitsubishiTemplate[15] = 0x00;
  }

  // KJ has a bit different template
  if (_mitsubishiModel == MITSUBISHI_KJ) {
    MitsubishiTemplate[15] = 0x00;
  }

  // Set the operatingmode on the template message
  MitsubishiTemplate[5] = powerMode;
  MitsubishiTemplate[6] = operatingMode;

  // Set the temperature on the template message
  if (temperature == 10) {
    Serial.println(F("Temp=10 maintenance mode"));
    MitsubishiTemplate[7] = 0x00; // Maintenance mode
    MitsubishiTemplate[15] = 0x20; // This seems to be set to 0x20 on maintenance mode
  } else {
    MitsubishiTemplate[7] = temperature - 16;
  }

  // Set the horizontal air direction on the template message
  MitsubishiTemplate[8] = swingH;

  // Set the fan speed and vertical air direction on the template message
  MitsubishiTemplate[9] = fanSpeed | swingV;



  if (_mitsubishiModel == MITSUBISHI_KJ) {
    MitsubishiTemplate[8] = 0;
    if ( operatingMode == MITSUBISHI_AIRCON1_MODE_AUTO || operatingMode == MITSUBISHI_AIRCON1_MODE_COOL )
      MitsubishiTemplate[8] = 0x6;
    if ( operatingMode == MITSUBISHI_AIRCON1_MODE_DRY )
      MitsubishiTemplate[8] = 0x2;

    if ( swingH != 0 ) {
      MitsubishiTemplate[16] = 0x02;
    }
  }

#ifdef USE_TIME_H
  time(&now);
  timeinfo = localtime(&now);

  MitsubishiTemplate[10] = (uint8_t)(timeinfo->tm_hour * 60 + timeinfo->tm_min)/6;
#endif

#ifdef IR_SEND_TIME
  if (_mitsubishiModel == MITSUBISHI_KJ) {
#ifdef DEBUG
    Serial.printf("Send time %02d:%02d day %d --> %x\n", sendHour, sendMinute, sendWeekday, (sendHour * 60 + sendMinute)/10);
#endif
    MitsubishiTemplate[10] = (uint8_t)((sendHour * 60 + sendMinute)/10);
    // Sunday is start if week , value 1
    MitsubishiTemplate[14] = TempTranslate[( sendWeekday - 1 ) % 7];
  }
  else {
#ifdef DEBUG
    Serial.printf("Send time %02d:%02d --> %x\n", sendHour, sendMinute, (sendHour * 60 + sendMinute)/6);
#endif
    MitsubishiTemplate[10] = (uint8_t)((sendHour * 60 + sendMinute)/6);
  }
#endif

#ifdef IR_DEBUG_PACKET
  IRPacket[0] = (char) 0;
  // Calculate the checksum
  for (int i=0; i<17; i++) {
    checksum += MitsubishiTemplate[i];
    sprintf_P(pbyte, PSTR("[%02d]=%02x "), i, (int) MitsubishiTemplate[i]);
    strcat(IRPacket, pbyte);
  }
  Serial.println(IRPacket);
#else
  // Calculate the checksum
  for (int i=0; i<17; i++) {
    checksum += MitsubishiTemplate[i];
  }
#endif

  MitsubishiTemplate[17] = checksum;

  // 40 kHz PWM frequency
  IR.setFrequency(38);

  // The Mitsubishi data is repeated twice
  for (int j=0; j<2; j++) {
    // Header
    IR.mark(MITSUBISHI_AIRCON1_HDR_MARK);
    IR.space(MITSUBISHI_AIRCON1_HDR_SPACE);

    // Data
    for (unsigned int i=0; i<sizeof(MitsubishiTemplate); i++) {
      IR.sendIRbyte(MitsubishiTemplate[i], MITSUBISHI_AIRCON1_BIT_MARK, MITSUBISHI_AIRCON1_ZERO_SPACE, MITSUBISHI_AIRCON1_ONE_SPACE);
    }

    // Pause between the first and the second data burst
    // Also modify one byte for the second burst on MSY model. This does not affect the checksum of the second burst
    if (j == 0) {
      IR.mark(MITSUBISHI_AIRCON1_BIT_MARK);
      IR.space(MITSUBISHI_AIRCON1_MSG_SPACE);

      if (_mitsubishiModel == MITSUBISHI_MSY) {
        MitsubishiTemplate[14] = 0x24;
      }
    }
  }

  // End mark
  IR.mark(MITSUBISHI_AIRCON1_BIT_MARK);
  IR.space(0);
}
