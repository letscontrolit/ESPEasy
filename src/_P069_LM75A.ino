#ifdef USES_P069
//#######################################################################################################
//########################### Plugin 69: LM75A Temperature Sensor (I2C) #################################
//#######################################################################################################
//###################### Library source code for Arduino by QuentinCG, 2016 #############################
//#######################################################################################################
//##################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ###############################
//############################## http://www.beicelectronics.com #########################################
//#######################################################################################################
//########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
//#######################################################################################################


#define PLUGIN_069
#define PLUGIN_ID_069         69
#define PLUGIN_NAME_069       "Environment - LM75A"
#define PLUGIN_VALUENAME1_069 "Temperature"


#ifndef LM75A_h
#define LM75A_h

#define INVALID_LM75A_TEMPERATURE 1000

namespace LM75AConstValues
{
  const int LM75A_BASE_ADDRESS = 0x48;
  const float LM75A_DEGREES_RESOLUTION = 0.125;
  const int LM75A_REG_ADDR_TEMP = 0;
}

using namespace LM75AConstValues;

class LM75A
{
public:
  LM75A(bool A0_value = false, bool A1_value = false, bool A2_value = false)
  {
    _i2c_device_address = LM75A_BASE_ADDRESS;

    if (A0_value) {
      _i2c_device_address += 1;
    }

    if (A1_value) {
      _i2c_device_address += 2;
    }

    if (A2_value) {
      _i2c_device_address += 4;
    }

    //Wire.begin();   called in ESPEasy framework
  }

  LM75A(uint8_t addr)
  {
    _i2c_device_address = addr;
    //Wire.begin();   called in ESPEasy framework
  }

  float getTemperatureInDegrees() const
  {
    float real_result = INVALID_LM75A_TEMPERATURE;
    int16_t value = 0;

    // Go to temperature data register
    Wire.beginTransmission(_i2c_device_address);
    Wire.write(LM75A_REG_ADDR_TEMP);
    if (Wire.endTransmission())
    {
      // Transmission error
      return real_result;
    }

    // Get content
    Wire.requestFrom(_i2c_device_address, (uint8_t)2);
    if (Wire.available() == 2)
    {
      value = (Wire.read() << 8) | Wire.read();
    }
    else
    {
      // Can't read temperature
      return real_result;
    }

    // Shift data (left-aligned)
    value >>= 5;

    // Relocate negative bit (11th bit to 16th bit)
    if (value & 0x0400)   // negative?
    {
      value |= 0xFC00;   // expand to 16 bit
    }

    // Real value can be calculated with sensor resolution
    real_result = (float)value * LM75A_DEGREES_RESOLUTION;

    return real_result;
  }

private:
  uint8_t _i2c_device_address;
};

#endif

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

LM75A* PLUGIN_069_LM75A = NULL;


boolean Plugin_069(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number = PLUGIN_ID_069;
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
      string = F(PLUGIN_NAME_069);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_069));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      int optionValues[8] = { 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };
      addFormSelectorI2C(F("i2c_addr"), 8, optionValues, CONFIG(0));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      CONFIG(0) = getFormItemInt(F("i2c_addr"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (PLUGIN_069_LM75A)
        delete PLUGIN_069_LM75A;
      PLUGIN_069_LM75A = new LM75A((uint8_t)CONFIG(0));

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (!PLUGIN_069_LM75A)
        return success;

      float tempC = PLUGIN_069_LM75A->getTemperatureInDegrees();

      if (tempC == INVALID_LM75A_TEMPERATURE)
      {
        String log = F("LM75A: No reading!");
        addLog(LOG_LEVEL_INFO, log);
        UserVar[event->BaseVarIndex] = NAN;
      }
      else
      {
        UserVar[event->BaseVarIndex] = tempC;
        String log = F("LM75A: Temperature: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P069
