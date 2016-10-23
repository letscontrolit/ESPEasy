//#######################################################################################################
//######################## Plugin 015 TSL2561 I2C Lux Sensor ############################################
//#######################################################################################################
// 13-10-2015 Charles-Henri Hallard, see my projects and blog at https://hallard.me

#define PLUGIN_015
#define PLUGIN_ID_015        15
#define PLUGIN_NAME_015       "Luminosity - TLS2561"
#define PLUGIN_VALUENAME1_015 "Lux"

boolean Plugin_015_init = false;

// ======================================
// TSL2561 luminosity sensor
// ======================================
#define TSL2561_I2C_ADDRESS 0x39 // I2C address for the sensor
#define TSL2561_CONTROL     0x80
#define TSL2561_TIMING      0x81
#define TSL2561_INTERRUPT   0x86
#define TSL2561_CHANNEL_0L  0x8C
#define TSL2561_CHANNEL_0H  0x8D
#define TSL2561_CHANNEL_1L  0x8E
#define TSL2561_CHANNEL_1H  0x8F

// Control register bits
#define TSL2561_POWER_UP   0x03
#define TSL2561_POWER_DOWN 0x00

// Timing register bits
#define TSL2561_TIMING_13MS         0x00
#define TSL2561_TIMING_101MS        0x01
#define TSL2561_TIMING_402MS        0x02
#define TSL2561_TIMING_CUSTOM_STOP  0x03
#define TSL2561_TIMING_CUSTOM_START 0x0B

#define TSL2561_LUX_SCALE     14     // scale by 2^14
#define TSL2561_RATIO_SCALE   9      // scale ratio by 2^9
#define TSL2561_CH_SCALE      10     // scale channel values by 2^10
#define TSL2561_CHSCALE_TINT_13MS  0x7517 // 322/11 * 2^CH_SCALE (13ms)
#define TSL2561_CHSCALE_TINT_60MS  0x1800 // 322/48 * 2^CH_SCALE (60ms)
#define TSL2561_CHSCALE_TINT_101MS 0x0fe7 // 322/81 * 2^CH_SCALE (101ms)
#define TSL2561_CHSCALE_TINT_120MS 0x0D6B // 322/96 * 2^CH_SCALE (120ms)
#define TSL2561_CHSCALE_TINT_402MS (1 << TSL2561_CH_SCALE) // default No scaling

// Clipping thresholds
#define TSL2561_CLIPPING_13MS     (4900)
#define TSL2561_CLIPPING_101MS    (37000)
#define TSL2561_CLIPPING_402MS    (65000)

#define TSL2561_K1T 0x0040   // 0.125 * 2^RATIO_SCALE
#define TSL2561_B1T 0x01f2   // 0.0304 * 2^LUX_SCALE
#define TSL2561_M1T 0x01be   // 0.0272 * 2^LUX_SCALE
#define TSL2561_K2T 0x0080   // 0.250 * 2^RATIO_SCA
#define TSL2561_B2T 0x0214   // 0.0325 * 2^LUX_SCALE
#define TSL2561_M2T 0x02d1   // 0.0440 * 2^LUX_SCALE
#define TSL2561_K3T 0x00c0   // 0.375 * 2^RATIO_SCALE
#define TSL2561_B3T 0x023f   // 0.0351 * 2^LUX_SCALE
#define TSL2561_M3T 0x037b   // 0.0544 * 2^LUX_SCALE
#define TSL2561_K4T 0x0100   // 0.50 * 2^RATIO_SCALE
#define TSL2561_B4T 0x0270   // 0.0381 * 2^LUX_SCALE
#define TSL2561_M4T 0x03fe   // 0.0624 * 2^LUX_SCALE
#define TSL2561_K5T 0x0138   // 0.61 * 2^RATIO_SCALE
#define TSL2561_B5T 0x016f   // 0.0224 * 2^LUX_SCALE
#define TSL2561_M5T 0x01fc   // 0.0310 * 2^LUX_SCALE
#define TSL2561_K6T 0x019a   // 0.80 * 2^RATIO_SCALE
#define TSL2561_B6T 0x00d2   // 0.0128 * 2^LUX_SCALE
#define TSL2561_M6T 0x00fb   // 0.0153 * 2^LUX_SCALE
#define TSL2561_K7T 0x029a   // 1.3 * 2^RATIO_SCALE
#define TSL2561_B7T 0x0018   // 0.00146 * 2^LUX_SCALE
#define TSL2561_M7T 0x0012   // 0.00112 * 2^LUX_SCALE
#define TSL2561_K8T 0x029a   // 1.3 * 2^RATIO_SCALE
#define TSL2561_B8T 0x0000   // 0.000 * 2^LUX_SCALE
#define TSL2561_M8T 0x0000   // 0.000 * 2^LUX_SCALE


uint16_t  tsl2561_lux; // latest lux value read

boolean Plugin_015(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_015;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_015);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_015));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        #define TLS2561_INTEGRATION_OPTION 3

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[TLS2561_INTEGRATION_OPTION];
        int optionValues[TLS2561_INTEGRATION_OPTION];
        optionValues[0] = TSL2561_TIMING_13MS;
        options[0] = F("13 ms");
        optionValues[1] = TSL2561_TIMING_101MS;
        options[1] = F("101 ms");
        optionValues[2] = TSL2561_TIMING_402MS;
        options[2] = F("402 ms");

        string += F("<TR><TD>Integration time:<TD><select name='plugin_015_integration'>");
        for (byte x = 0; x < TLS2561_INTEGRATION_OPTION; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_015_integration");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        Plugin_015_init = false; // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Get sensor resolution configuration
        uint8_t integration = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        uint8_t ret;

        if (!Plugin_015_init) {
          Plugin_015_init = Plugin_015_tls2561_begin(integration);
        }

        // Read values if init ok
        if (Plugin_015_init) {
          ret = Plugin_015_tsl2561_calcLux(integration);
          if (ret == 0) {
            UserVar[event->BaseVarIndex] = tsl2561_lux;
            success = true;
            String log = F("TLS2561 : Lux: ");
            log += UserVar[event->BaseVarIndex];
            addLog(LOG_LEVEL_INFO,log);
          } else {
            String log = F("TLS2561 : Read Error #");
            log += String(ret,DEC);
            addLog(LOG_LEVEL_INFO,log);
          }
        }
        break;
      }

  }
  return success;
}

/* ======================================================================
Function: Plugin_015_tls2561_begin
Purpose : read the user register from the sensor
Input   : integration time
Output  : true if okay
Comments: -
====================================================================== */
boolean Plugin_015_tls2561_begin(uint8_t integration)
{
  uint8_t ret;

  // Power UP device
  ret = Plugin_015_tsl2561_writeRegister(TSL2561_CONTROL, TSL2561_POWER_UP); 
  if ( ret == 0 )
  {
    // I noticed 1st calculation after power up could be hazardous; so
    // do a 1st dummy reading, with speed integration time, here 13ms
    Plugin_015_tsl2561_writeRegister(TSL2561_TIMING, TSL2561_TIMING_13MS);  
    delay(15);  
    ret = true;
  } else {
    String log = F("TLS2561 : integration=0x");
    log += String(integration,HEX);
    log += F(" => Error 0x");
    log += String(ret,HEX);
    addLog(LOG_LEVEL_INFO,log);
    ret = false;
  }

  return ret; 
}

/* ======================================================================
Function: Plugin_015_tsl2561_readRegister
Purpose : read a register from the sensor
Input   : register address
          register value filled by function
Output  : 0 if okay
Comments: -
====================================================================== */
uint8_t Plugin_015_tsl2561_readRegister(uint8_t reg, uint8_t * value)
{
  Wire.beginTransmission(TSL2561_I2C_ADDRESS);
  Wire.write(reg);         
  // all was fine ?
  if ( Wire.endTransmission()==0 ) {
    // request 1 byte and have it ?
    if (Wire.requestFrom(TSL2561_I2C_ADDRESS, 1)==1) {
      // return value
      *value = Wire.read();
      return 0;
    }
  }
  return 1;
}

/* ======================================================================
Function: Plugin_015_tsl2561_writeRegister
Purpose : read a register from the sensor
Input   : register address
Output  : register value
Comments: 0 if okay
====================================================================== */
uint8_t Plugin_015_tsl2561_writeRegister(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(TSL2561_I2C_ADDRESS);
  Wire.write(reg);
  Wire.write(value); 
  return (Wire.endTransmission()); 
}

/* ======================================================================
Function: Plugin_015_tsl2561_calcLux
Purpose : start a conversion and return calculated lux
Input   : integration time
Output  : 0 if calculated value ok and updated 
Comments: global lux value is updated
====================================================================== */
int8_t Plugin_015_tsl2561_calcLux(uint8_t integration)
{
  unsigned long chScale;
  unsigned long channel0, channel1;
  unsigned long ratio, ratio1;
  unsigned long lux;
  unsigned int b, m;
  uint16_t ch0,ch1;
  uint16_t clipThreshold;
  uint8_t msb, lsb;
  uint8_t err = 0;

  // do start calculation with speed integration time, 
  Plugin_015_tsl2561_writeRegister(TSL2561_TIMING, integration);  
  if (integration == TSL2561_TIMING_402MS ) {
    chScale = TSL2561_CHSCALE_TINT_402MS ;
    clipThreshold = TSL2561_CLIPPING_402MS ;
    delay(405);
  } else if (integration == TSL2561_TIMING_101MS ) {
    chScale = TSL2561_CHSCALE_TINT_101MS ;
    clipThreshold = TSL2561_CLIPPING_101MS ;
    delay(103);
  } else {
    chScale = TSL2561_CHSCALE_TINT_13MS ;
    clipThreshold = TSL2561_CLIPPING_13MS ;
    delay(15);
  }

  // don't try to change reading order of LOW/HIGH, it will not work !!!!
  // you must read LOW then HIGH
  err |= Plugin_015_tsl2561_readRegister(TSL2561_CHANNEL_0L, &lsb);
  err |= Plugin_015_tsl2561_readRegister(TSL2561_CHANNEL_0H, &msb);
  ch0 = word(msb,lsb);
  err |= Plugin_015_tsl2561_readRegister(TSL2561_CHANNEL_1L, &lsb);
  err |= Plugin_015_tsl2561_readRegister(TSL2561_CHANNEL_1H, &msb);
  ch1 = word(msb,lsb);;

  // I2C error ?
  if( err )
    return -2; 

  /* Sensor saturated the lux is not valid in this situation */
  if ((ch0 > clipThreshold) || (ch1 > clipThreshold))
  {
    return -1;
  }

  // gain is 1 so put it to 16X
  chScale <<= 4;
  
  // scale the channel values
  channel0 = (ch0 * chScale) >> TSL2561_CH_SCALE;
  channel1 = (ch1 * chScale) >> TSL2561_CH_SCALE;

  ratio1 = 0;
  if (channel0!= 0) 
    ratio1 = (channel1 << (TSL2561_RATIO_SCALE+1))/channel0;
  
  // round the ratio value
  ratio = (ratio1 + 1) >> 1;

  // ULPNode have T package
  // Adjust constant depending on calculated ratio
  if ((ratio >= 0) && (ratio <= TSL2561_K1T))
    {b=TSL2561_B1T; m=TSL2561_M1T;}
  else if (ratio <= TSL2561_K2T)
    {b=TSL2561_B2T; m=TSL2561_M2T;}
  else if (ratio <= TSL2561_K3T)
    {b=TSL2561_B3T; m=TSL2561_M3T;}
  else if (ratio <= TSL2561_K4T)
    {b=TSL2561_B4T; m=TSL2561_M4T;}
  else if (ratio <= TSL2561_K5T)
    {b=TSL2561_B5T; m=TSL2561_M5T;}
  else if (ratio <= TSL2561_K6T)
    {b=TSL2561_B6T; m=TSL2561_M6T;}
  else if (ratio <= TSL2561_K7T)
    {b=TSL2561_B7T; m=TSL2561_M7T;}
  else if (ratio > TSL2561_K8T)
    {b=TSL2561_B8T; m=TSL2561_M8T;}

  // datasheet formula
  lux=((channel0*b)-(channel1*m));
  
  // do not allow negative lux value
  if(lux<0) 
    lux=0;
  
  // round lsb (2^(LUX_SCALE−1))
  lux += (1<<(TSL2561_LUX_SCALE-1));
  
  // strip off fractional portion
  lux >>= TSL2561_LUX_SCALE;

  // strip off fractional portion
  tsl2561_lux = (uint16_t) (lux);

  return 0;
}
