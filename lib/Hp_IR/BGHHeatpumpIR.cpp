#include <BGHHeatpumpIR.h>

BGHHeatpumpIR::BGHHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "BGH_aud";
  static const char info[]  PROGMEM = "{\"mdl\":\"BGH_aud\",\"dn\":\"BGH AUD\",\"mT\":18,\"xT\":32,\"fs\":3}";

  _model = model;
  _info = info;
}


void BGHHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd , uint8_t fanSpeedCmd , uint8_t temperatureCmd , uint8_t swingVCmd , uint8_t swingHCmd )
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = BGH_AIRCON1_POWER_ON;
  uint8_t operatingMode = BGH_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = BGH_AIRCON1_FAN_AUTO;
  uint8_t temperature = 24;
  uint8_t swingV=0;
  uint8_t swingH=0;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = BGH_AIRCON1_POWER_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = BGH_AIRCON1_MODE_AUTO;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in AUTO mode
        break;
      case MODE_HEAT:
        operatingMode = BGH_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = BGH_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = BGH_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = BGH_AIRCON1_MODE_FAN;
        if ( fanSpeedCmd == FAN_AUTO ) {
          fanSpeedCmd = FAN_1; // Fan speed cannot be 'AUTO' in FAN mode
          temperatureCmd = 25; // Fixed temperature FAN mode
        }
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = BGH_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = BGH_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = BGH_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = BGH_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 17 && temperatureCmd < 33)
  {
    temperature = temperatureCmd;
  }


  sendBGH(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

// Send the BGH code
void BGHHeatpumpIR::sendBGH(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV ,uint8_t swingH)
{
  (void)swingV;
  (void)swingH;

  uint8_t BGHTemplate[] = { 0x83, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,    // Header uint8_t 0-1
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //

  uint8_t  i;

  // Set the Fan speed, On/Off.
  BGHTemplate[2] = fanSpeed | powerMode;
  BGHTemplate[3] =  (((temperature - 18)  << 4)  | operatingMode ) ;

  // Calculate the uint8_t checksum EXOR uint8_t 2 to 12
  BGHTemplate[13] = BGHTemplate[2];
  for (i=3; i<13; i++) {
     BGHTemplate[13]= BGHTemplate[i] ^ BGHTemplate[13];
  }

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(BGH_AIRCON1_HDR_MARK);
  IR.space(BGH_AIRCON1_HDR_SPACE);

  // Payload header part
  for (i=0; i<7; i++) {
    IR.sendIRbyte(BGHTemplate[i], BGH_AIRCON1_BIT_MARK, BGH_AIRCON1_ZERO_SPACE, BGH_AIRCON1_ONE_SPACE);
  }

  // Mesage space
  IR.mark(BGH_AIRCON1_BIT_MARK);
  IR.space(BGH_AIRCON1_MSG_SPACE);

  // Payload message part
  for (; i<14; i++) {
    IR.sendIRbyte(BGHTemplate[i], BGH_AIRCON1_BIT_MARK, BGH_AIRCON1_ZERO_SPACE, BGH_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(BGH_AIRCON1_BIT_MARK);
  IR.space(0);
}