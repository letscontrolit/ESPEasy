//#######################################################################################################
//#################### Plugin 051 Temperature and Humidity Sensor AM2320 ##############
//#######################################################################################################
//
// Temperature and Humidity Sensor AM2320
// written by https://github.com/krikk
// based on this library: https://github.com/hibikiledo/AM2320
// this code is based on version 1.0 of the above library
//

#ifdef PLUGIN_BUILD_TESTING

#include <AM2320.h>

#define PLUGIN_051
#define PLUGIN_ID_051        51
#define PLUGIN_NAME_051       "Environment - AM2320 [TEST]"
#define PLUGIN_VALUENAME1_051 "Temperature"
#define PLUGIN_VALUENAME2_051 "Humidity"




boolean Plugin_051(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_051;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_051);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_051));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_051));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
      	AM2320 sensor;
      	sensor.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);

        // sensor.measure() returns boolean value
        // - true indicates measurement is completed and success
        // - false indicates that either sensor is not ready or crc validation failed
        //   use getErrorCode() to check for cause of error.
        if (sensor.measure()) {
        	UserVar[event->BaseVarIndex] = sensor.getTemperature();
        	UserVar[event->BaseVarIndex + 1] = sensor.getHumidity();

        	String log = F("AM2320: Temperature: ");
        	log += UserVar[event->BaseVarIndex];
        	addLog(LOG_LEVEL_INFO, log);
        	log = F("AM2320: Humidity: ");
        	log += UserVar[event->BaseVarIndex + 1];
        	addLog(LOG_LEVEL_INFO, log);
        	success = true;
        	break;
        }
        else {  // error has occured
          int errorCode = sensor.getErrorCode();
          switch (errorCode) {
            case 1: addLog(LOG_LEVEL_ERROR, F("AM2320: Sensor is offline")); break;
            case 2: addLog(LOG_LEVEL_ERROR, F("AM2320: CRC validation failed.")); break;
          }
          success = false;
          break;
        }
      }
  }
  return success;
}


#endif
