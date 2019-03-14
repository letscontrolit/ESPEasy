#ifdef USES_P083
/*********************************************************************************************\
 * SGP30 - Gas (TVOC - Total Volatile Organic Compounds) and Air Quality (eCO2)
 *
 *
 * I2C Address: 0x58
\*********************************************************************************************/
#include "Adafruit_SGP30.h"


#define PLUGIN_083
#define PLUGIN_ID_083        83
#define PLUGIN_NAME_083       "Gas - SGP30 [TESTING]"
#define PLUGIN_VALUENAME1_083 "TVOC"
#define PLUGIN_VALUENAME2_083 "eCO2"

boolean Plugin_083(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  switch (function)
    {
      case PLUGIN_DEVICE_ADD:
        {
          Device[++deviceCount].Number = PLUGIN_ID_083;
          Device[deviceCount].Type = DEVICE_TYPE_I2C;
          Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
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
          string = F(PLUGIN_NAME_083);
          break;
        }
  
      case PLUGIN_GET_DEVICEVALUENAMES:
        {
          strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_083));
          strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_083));
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
          Adafruit_SGP30 sgp;
          if (sgp.begin()) 
          {
              if (sgp.IAQmeasure()) 
              {
                  UserVar[event->BaseVarIndex] = sgp.TVOC;
                  UserVar[event->BaseVarIndex + 1] = sgp.eCO2;
                  String log = F("SGP30: TVOC: ");
                  log += UserVar[event->BaseVarIndex];
                  addLog(LOG_LEVEL_INFO, log);
                  log = F("SGP30: eCO2: ");
                  log += UserVar[event->BaseVarIndex + 1];
                  addLog(LOG_LEVEL_INFO, log);
                  success = true;
                  break;
               }else{
                  addLog(LOG_LEVEL_ERROR, F("SGP30: Measurement failed"))
                  break;
               }
           }else{
              addLog(LOG_LEVEL_ERROR, F("SGP30: Sensor not found"))
              break;
           } 
        }
    }
    return success;
}


#endif // USES_P083
