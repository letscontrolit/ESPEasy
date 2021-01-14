#include "_Plugin_Helper.h"
#ifdef USES_P072

// ######################################################################################################
// ####################### Plugin 072: Temperature and Humidity sensor HDC1080 (I2C) ####################
// ######################################################################################################


#define PLUGIN_072
#define PLUGIN_ID_072         72
#define PLUGIN_NAME_072       "Environment - HDC1080 (I2C) [TESTING]"
#define PLUGIN_VALUENAME1_072 "Temperature"
#define PLUGIN_VALUENAME2_072 "Humidity"


#define HDC1080_I2C_ADDRESS      0x40 // I2C address for the sensor

boolean Plugin_072(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_072;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_072);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_072));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_072));
      break;
    }

    case PLUGIN_READ:
    {
      byte hdc1080_msb, hdc1080_lsb;
      uint16_t hdc1080_rawtemp, hdc1080_rawhum;
      float    hdc1080_temp, hdc1080_hum;

      Wire.beginTransmission(HDC1080_I2C_ADDRESS); // start transmission to device
      Wire.write(0x02);                            // sends HDC1080_CONFIGURATION
      Wire.write(0b00000000);                      // set resolution to 14bits both for T and H
      Wire.write(0x00);                            // **reserved**
      Wire.endTransmission();                      // end transmission
      delay(10);

      Wire.beginTransmission(HDC1080_I2C_ADDRESS); // start transmission to device
      Wire.write(0x00);                            // sends HDC1080_TEMPERATURE
      Wire.endTransmission();                      // end transmission
      delay(9);
      Wire.requestFrom(HDC1080_I2C_ADDRESS, 2);    // read 2 bytes for temperature
      hdc1080_msb     = Wire.read();
      hdc1080_lsb     = Wire.read();
      hdc1080_rawtemp = hdc1080_msb << 8 | hdc1080_lsb;
      hdc1080_temp    = (hdc1080_rawtemp / pow(2, 16)) * 165 - 40;

      Wire.beginTransmission(HDC1080_I2C_ADDRESS); // start transmission to device
      Wire.write(0x01);                            // sends HDC1080_HUMIDITY
      Wire.endTransmission();                      // end transmission
      delay(9);
      Wire.requestFrom(HDC1080_I2C_ADDRESS, 2);    // read 2 bytes for humidity
      hdc1080_msb    = Wire.read();
      hdc1080_lsb    = Wire.read();
      hdc1080_rawhum = hdc1080_msb << 8 | hdc1080_lsb;
      hdc1080_hum    = (hdc1080_rawhum / pow(2, 16)) * 100;

      UserVar[event->BaseVarIndex]     = hdc1080_temp;
      UserVar[event->BaseVarIndex + 1] = hdc1080_hum;
      
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("HDC1080: Temperature: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        addLog(LOG_LEVEL_INFO, log);
        log  = F("HDC1080: Humidity: ");
        log += formatUserVarNoCheck(event->TaskIndex, 1);
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P072
