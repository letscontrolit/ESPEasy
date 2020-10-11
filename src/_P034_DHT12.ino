#ifdef USES_P034

// #######################################################################################################
// ######################## Plugin 034: Temperature and Humidity sensor DHT 12 (I2C) #####################
// #######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_034
#define PLUGIN_ID_034         34
#define PLUGIN_NAME_034       "Environment - DHT12 (I2C)"
#define PLUGIN_VALUENAME1_034 "Temperature"
#define PLUGIN_VALUENAME2_034 "Humidity"

#define DHT12_I2C_ADDRESS      0x5C // I2C address for the sensor

boolean Plugin_034(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_034;
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
      string = F(PLUGIN_NAME_034);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_034));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_034));
      break;
    }

    case PLUGIN_READ:
    {
      byte dht_dat[5];

      // byte dht_in;
      byte i;

      // byte Retry = 0;
      boolean error = false;

      Wire.beginTransmission(DHT12_I2C_ADDRESS);         // start transmission to device
      Wire.write(0);                                     // sends register address to read from
      Wire.endTransmission();                            // end transmission

      if (Wire.requestFrom(DHT12_I2C_ADDRESS, 5) == 5) { // send data n-bytes read
        for (i = 0; i < 5; i++)
        {
          dht_dat[i] = Wire.read();                      // receive DATA
        }
      } else {
        error = true;
      }

      if (!error)
      {
        // Checksum calculation is a Rollover Checksum by design!
        byte dht_check_sum = dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]; // check check_sum

        if (dht_dat[4] == dht_check_sum)
        {
          float temperature = float(dht_dat[2] * 10 + (dht_dat[3] & 0x7f)) / 10.0f; // Temperature

          if (dht_dat[3] & 0x80) { temperature = -temperature; }
          float humidity = float(dht_dat[0] * 10 + dht_dat[1]) / 10.0f;             // Humidity

          UserVar[event->BaseVarIndex]     = temperature;
          UserVar[event->BaseVarIndex + 1] = humidity;
          String log = F("DHT12: Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          log  = F("DHT12: Humidity: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);

          /*
                      log = F("DHT12: Data: ");
                      for (int i=0; i < 5; i++)
                      {
                        log +=  dht_dat[i];
                        log += ", ";
                      }
                      addLog(LOG_LEVEL_INFO, log);
           */
          success = true;
        } // checksum
      }   // error

      if (!success)
      {
        addLog(LOG_LEVEL_INFO, F("DHT12: No reading!"));
        UserVar[event->BaseVarIndex]     = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P034
