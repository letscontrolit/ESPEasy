#include "ElectroluxHeatpumpIR.h"

// This is a protected method, i.e. generic Electrolux instances cannot be created
ElectroluxHeatpumpIR::ElectroluxHeatpumpIR() : HeatpumpIR()
{
}

// Support for YAL1F remote
ElectroluxYALHeatpumpIR::ElectroluxYALHeatpumpIR() : ElectroluxHeatpumpIR()
{
  static const char model[] PROGMEM = "electroluxyal";
  static const char info[]  PROGMEM = "{\"mdl\":\"electroluxyal\",\"dn\":\"Electrolux YAL\",\"mT\":16,\"xT\":30,\"fs\":3}";

  _model = model;
  _info = info;
  electroluxModel = ELECTROLUX_YAL;
}

void ElectroluxHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  (void)swingVCmd;
  (void)swingHCmd;

  // Sensible defaults for the heat pump mode

  uint8_t powerMode = ELECTROLUX_AIRCON1_POWER_ON;
  uint8_t operatingMode = ELECTROLUX_AIRCON1_MODE_HEAT;
  uint8_t fanSpeed = ELECTROLUX_AIRCON1_FAN_AUTO;
  uint8_t temperature = 21;
  uint8_t swingV = ELECTROLUX_VDIR_AUTO;
  uint8_t swingH = ELECTROLUX_HDIR_AUTO;


  if (powerModeCmd == POWER_OFF)
  {
    powerMode = ELECTROLUX_AIRCON1_POWER_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = ELECTROLUX_AIRCON1_MODE_AUTO;
        temperatureCmd = 25;
        break;
      case MODE_HEAT:
        operatingMode = ELECTROLUX_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = ELECTROLUX_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = ELECTROLUX_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_1;
        break;
      case MODE_FAN:
        operatingMode = ELECTROLUX_AIRCON1_MODE_FAN;
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = ELECTROLUX_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = ELECTROLUX_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = ELECTROLUX_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = ELECTROLUX_AIRCON1_FAN3;
      break;
  }

  switch (swingVCmd)
  {
    case VDIR_AUTO:
        swingV = ELECTROLUX_VDIR_AUTO;
        break;
    case VDIR_SWING:
      swingV = ELECTROLUX_VDIR_SWING;
      break;
  }

  switch (swingHCmd)
  {
    case HDIR_AUTO:
      swingH = ELECTROLUX_HDIR_AUTO;
      break;
    case HDIR_SWING:
      swingH = ELECTROLUX_HDIR_SWING;
      break;
  }
    
  


  if (temperatureCmd > 15 && temperatureCmd < 31)
  {
    temperature = temperatureCmd - 16;
  }

  sendElectrolux(IR, powerMode, operatingMode, fanSpeed, temperature, swingV, swingH);
}

// Send the Electrolux code
void ElectroluxHeatpumpIR::sendElectrolux(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{

  uint8_t ElectroluxTemplate[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00 };
  //                            0     1     2     3     4     5     6     7
  uint8_t i;

  // Set the Fan speed, operating mode and power state
  ElectroluxTemplate[0] = fanSpeed | operatingMode | powerMode;
  // Set the temperature
  ElectroluxTemplate[1] = temperature;
  
    ElectroluxTemplate[2] = 0x20; // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
    ElectroluxTemplate[3] = 0x50; // bits 4..7 always 0101
    ElectroluxTemplate[6] = 0x00;

    if (swingV == ELECTROLUX_VDIR_SWING)
    {
      ElectroluxTemplate[0] |= (1 << 6); // Enable swing by setting bit 6
      ElectroluxTemplate[4] |= 0x01; // Set vertical swing direction
    }
    if(swingH == ELECTROLUX_HDIR_SWING)
    {
      ElectroluxTemplate[0] |= (1 << 6); // Enable swing by setting bit 6
      ElectroluxTemplate[4] |= 0x10; // Set horizontal swing direction
    }

    ElectroluxTemplate[7] = (((
     (ElectroluxTemplate[0] & 0x0F) +
     (ElectroluxTemplate[1] & 0x0F) +
     (ElectroluxTemplate[2] & 0x0F) +
     (ElectroluxTemplate[3] & 0x0F) +
     ((ElectroluxTemplate[4] & 0xF0) >> 4) +
     ((ElectroluxTemplate[5] & 0xF0) >> 4) +
     ((ElectroluxTemplate[6] & 0xF0) >> 4) +
      0x0A) & 0x0F) << 4) | (ElectroluxTemplate[7] & 0x0F);

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Send Header mark
  IR.mark(ELECTROLUX_AIRCON1_HDR_MARK);
  IR.space(ELECTROLUX_AIRCON1_HDR_SPACE);

  // Payload part #1
  for (i=0; i<4; i++) {
    IR.sendIRbyte(ElectroluxTemplate[i], ELECTROLUX_AIRCON1_BIT_MARK, ELECTROLUX_AIRCON1_ZERO_SPACE, ELECTROLUX_AIRCON1_ONE_SPACE);
  }
  // Only three first bits of byte 4 are sent, this is always '010'
  IR.mark(ELECTROLUX_AIRCON1_BIT_MARK);
  IR.space(ELECTROLUX_AIRCON1_ZERO_SPACE);
  IR.mark(ELECTROLUX_AIRCON1_BIT_MARK);
  IR.space(ELECTROLUX_AIRCON1_ONE_SPACE);
  IR.mark(ELECTROLUX_AIRCON1_BIT_MARK);
  IR.space(ELECTROLUX_AIRCON1_ZERO_SPACE);

  // Message space
  IR.mark(ELECTROLUX_AIRCON1_BIT_MARK);
  IR.space(ELECTROLUX_AIRCON1_MSG_SPACE);

  // Payload part #2
  for (i=4; i<8; i++) {
    IR.sendIRbyte(ElectroluxTemplate[i], ELECTROLUX_AIRCON1_BIT_MARK, ELECTROLUX_AIRCON1_ZERO_SPACE, ELECTROLUX_AIRCON1_ONE_SPACE);
	}

  // End mark
  IR.mark(ELECTROLUX_AIRCON1_BIT_MARK);
  IR.space(0);
}
