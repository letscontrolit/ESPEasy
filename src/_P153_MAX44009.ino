//#######################################################################################################
//############################### Plugin 153: MAX44009 I2C 0x4A  #######################################
//#######################################################################################################

// based on :
// 1) https://github.com/RobTillaart/Arduino/tree/master/libraries/Max44009
// 2) https://github.com/dantudose/MAX44009/blob/master/MAX44009.cpp
//
// written by https://github.com/apszowski

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_153
#define PLUGIN_ID_153 153
#define PLUGIN_NAME_153 "Light/Lux - MAX44009 (GY-49)"
#define PLUGIN_VALUENAME1_153 "Lux"

boolean Plugin_153_init = false;

enum {
  //I2C address
  MAX44009_I2C_ADDR                     = 0x4A,
  // CONFIGURATION
  MAX44009_REGISTER_CONFIGURATION       = 0x02,
  MAX44009_CFG_MANUAL                   = 0x40,
  MAX44009_CFG_CONTINUOUS               = 0x80,
  //LUX READING
  MAX44009_REGISTER_LUX_HIGH            = 0x03,
  MAX44009_REGISTER_LUX_LOW             = 0x04,
};


uint16_t Plugin_153_readRegister(uint8_t reg) {
  uint16_t ret;
  Wire.beginTransmission(MAX44009_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(MAX44009_I2C_ADDR,1);
  ret = Wire.read();
  return ret;
}

void Plugin_153_writeRegister(uint8_t reg, uint8_t value){
  Wire.beginTransmission(MAX44009_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

float Plugin_153_readLux(void)
{
  uint8_t luxHigh = Plugin_153_readRegister(MAX44009_REGISTER_LUX_HIGH);
  uint8_t luxLow = Plugin_153_readRegister(MAX44009_REGISTER_LUX_LOW);
  uint8_t e = (luxHigh & 0xF0) >> 4;
  uint8_t m = (luxHigh & 0x0F) << 4 | luxLow;
  float lux = pow(2,e) * m * 0.045;
  return lux;
}
void Plugin_153_setModeAutomatic(void)
{
  uint8_t config = Plugin_153_readRegister(MAX44009_REGISTER_CONFIGURATION);
  config &= ~MAX44009_CFG_CONTINUOUS; // off
  config &= ~MAX44009_CFG_MANUAL;     // off
  Plugin_153_writeRegister(MAX44009_REGISTER_CONFIGURATION, config);
}

void Plugin_153_setModeContinuous(void)
{
  uint8_t config = Plugin_153_readRegister(MAX44009_REGISTER_CONFIGURATION);
  config |= MAX44009_CFG_CONTINUOUS; // on
  config &= ~MAX44009_CFG_MANUAL;    // off
  Plugin_153_writeRegister(MAX44009_REGISTER_CONFIGURATION, config);
}

void Plugin_153_setModeManual(uint8_t CDR, uint8_t TIM)
{
  uint8_t config = Plugin_153_readRegister(MAX44009_REGISTER_CONFIGURATION);
  config &= ~MAX44009_CFG_CONTINUOUS; // off
  config |= MAX44009_CFG_MANUAL;      // on
  config &= 0xF0; // clear CDR & TIM bits
  config |= CDR << 3 | TIM;
  Plugin_153_writeRegister(MAX44009_REGISTER_CONFIGURATION, config);
}

boolean Plugin_153(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_153;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_153);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_153));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice_mode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[8];
        int optionValues[8];
        options[0] = F("Automatic");
        optionValues[0] = 0;
        options[1] = F("Continuous");
        optionValues[1] = 1;
        options[2] = F("Manual");
        optionValues[2] = 2;
        // addFormSelector(string, F("Mode"), F("plugin_153_mode"), 3, options, optionValues, choice_mode);
        if( choice_mode != 0)
        {
          //########################
          byte choice_division = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          options[0] = F("Not divided");
          optionValues[0] = 0;
          options[1] = F("Divided 1/8");
          optionValues[1] = 1;
          // addFormSelector(string, F("Current Division Ratio"), F("plugin_153_divide"), 2, options, optionValues, choice_division);
          //########################
          byte choice_time = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          float integration_time = 800;
          for( uint8_t i = 0 ; i <= 7 ; i++)
          {
            options[i] =  integration_time;
            options[i] += F(" ms");
            optionValues[i] = i;
            integration_time = integration_time/2.0;
          }
          // addFormSelector(string, F("Integration Time"), F("plugin_153_time"), 8, options, optionValues, choice_time);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_153_mode"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_153_divide"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_153_time"));
        Plugin_153_init = false;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_153_init = true;
        uint8_t mode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        uint8_t divide = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        uint8_t tim = Settings.TaskDevicePluginConfig[event->TaskIndex][2];

        String log = F("MAX44009  :\rINIT MODE = ");
        if( mode == 0)
        {
          Plugin_153_setModeAutomatic();
          log += F("Automatic");
        }
        else if ( mode == 1)
        {
          Plugin_153_setModeContinuous();
          log += F("Continuous");
        }
        else
        {
          Plugin_153_setModeManual(divide,tim);
          log += F("Manual , CDR = ");
          log += divide;
          log += F(", Integration Time = ");
          log += tim;
        }
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = (float) Plugin_153_readLux();
        String log = F("MAX44009  : Ambient Light: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }
  }
  return success;
}

#endif
