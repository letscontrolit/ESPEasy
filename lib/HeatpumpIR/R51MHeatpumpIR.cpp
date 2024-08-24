#include <R51MHeatpumpIR.h>

R51MHeatpumpIR::R51MHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "R51M";
  static const char info[]  PROGMEM = "{}";
  
  _model = model;
  _info = info;
}


void R51MHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  const static byte tempMap [] PROGMEM = {0,1,3,2,6,7,5,4,12,13,9,8,10,11 };
  // Sensible defaults for the heat pump mode
  
  uint8_t data[] = { 0xB2, 0x0F, 0x00  }; // The actual data is in this part
  
  // Send the R51M code
  if (powerModeCmd == POWER_OFF)
  {
    data[1] = R51M_AIRCON1_MODE_OFF;
    data[2] = R51M_AIRCON1_MODE_OFF_TEMP;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        data[1] |= R51M_AIRCON1_FAN_DRY;
        data[2] |= R51M_AIRCON1_MODE_AUTO;
        break;
      case MODE_COOL:
        data[2] |= R51M_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        data[1] |= R51M_AIRCON1_FAN_DRY;
        data[2] |= R51M_AIRCON1_MODE_FANDRY;
        break;
      case MODE_FAN:
        data[2] |= R51M_AIRCON1_MODE_FANDRY;
        break;    
    }
    if (operatingModeCmd == MODE_COOL || operatingModeCmd == MODE_FAN)
    {
      switch (fanSpeedCmd)
      {
        case FAN_AUTO:
          data[1] |= R51M_AIRCON1_FAN_AUTO;
          break;
        case FAN_1:
          data[1] |= R51M_AIRCON1_FAN1;
          break;
        case FAN_2:
          data[1] |= R51M_AIRCON1_FAN2;
          break;
        case FAN_3:
          data[1] |= R51M_AIRCON1_FAN3;
          break;
      }
    }
    if (operatingModeCmd == MODE_COOL || operatingModeCmd == MODE_AUTO || operatingModeCmd == MODE_DRY)
    {
      if (temperatureCmd < 17 || temperatureCmd > 30) temperatureCmd = 24;
      data[2] |= (pgm_read_byte_near(tempMap + (temperatureCmd - 17)) << 4);
    }
    else data[2] |= R51M_AIRCON1_MODE_OFF_TEMP;
  }
   
  // 38 kHz PWM frequency
  IR.setFrequency(38);
  IR.space(R51M_AIRCON1_ZERO_SPACE);
  // Header
  IR.mark(R51M_AIRCON1_HDR_MARK-500); IR.space(R51M_AIRCON1_HDR_SPACE);

  // Payload data message part
  for (int i=0; i<3; i++) {
    IR.sendIRbyte(IR.bitReverse(data[i]), R51M_AIRCON1_BIT_MARK, R51M_AIRCON1_ZERO_SPACE, R51M_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(IR.bitReverse(~data[i]), R51M_AIRCON1_BIT_MARK, R51M_AIRCON1_ZERO_SPACE, R51M_AIRCON1_ONE_SPACE);
  }
 
  IR.mark(R51M_AIRCON1_BIT_MARK); IR.space(5000);
  // Header
  IR.mark(R51M_AIRCON1_HDR_MARK); IR.space(R51M_AIRCON1_HDR_SPACE);
  
  for (int i=0; i<3; i++) {
    IR.sendIRbyte(IR.bitReverse(data[i]), R51M_AIRCON1_BIT_MARK, R51M_AIRCON1_ZERO_SPACE, R51M_AIRCON1_ONE_SPACE);
    IR.sendIRbyte(IR.bitReverse(~data[i]), R51M_AIRCON1_BIT_MARK, R51M_AIRCON1_ZERO_SPACE, R51M_AIRCON1_ONE_SPACE);
  }
 
  // End mark
  IR.mark(R51M_AIRCON1_BIT_MARK);
  IR.space(0);
}

