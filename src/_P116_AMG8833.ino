#include "_Plugin_Helper.h"
#ifdef USES_P116

// #######################################################################################################
// ############################# Plugin 116: AMG8833 IR temperature I2C 0x69 #############################
// #######################################################################################################


//#include "src/PluginStructs/P116_data_struct.h
#include <Adafruit_AMG88xx.h>
#define DISPLAY_MATRIX true //display the temperature matrix

// Default I2C Address of the sensor
#define DEFAULT_ADDR 0x69

#define PLUGIN_116
#define PLUGIN_ID_116 116
#define PLUGIN_NAME_116 "IR Temperature CAM - AMG88xx"
#define PLUGIN_VALUENAME1_116 "Temp.Min"
#define PLUGIN_VALUENAME2_116 "Temp.Max"
#define PLUGIN_VALUENAME3_116 "Temp.Ave"

#define P116_I2C_ADDR       PCONFIG(0)

Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

boolean PLUGIN_116(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_116;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_116);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_116));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_116));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_116));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"),
                     formatToHex_decimal(P116_I2C_ADDR), 4);

      // FIXME TD-er: Why not using addFormSelectorI2C here?
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {

      addFormCheckBox(F("Change Sensor address"), F("p047_changeAddr"), false);
      addFormTextBox(F("Change I2C Addr. to (Hex)"), F("p047_i2cSoilMoisture_changeAddr"),
                     formatToHex_decimal(P047_I2C_ADDR), 4);

      addFormSeparator(2);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String plugin1 = webArg(F("i2c_addr"));
      P047_I2C_ADDR = (int)strtol(plugin1.c_str(), 0, 16);

      P047_SENSOR_SLEEP = isFormItemChecked(F("p047_sleep"));

      P047_CHECK_VERSION = isFormItemChecked(F("p047_version"));

      String plugin4 = webArg(F("p047_i2cSoilMoisture_changeAddr"));
      P047_NEW_ADDR = (int)strtol(plugin4.c_str(), 0, 16);

      P047_CHANGE_ADDR = isFormItemChecked(F("p047_changeAddr"));
      success          = true;
      break;
    }

   case PLUGIN_WEBFORM_LOAD:
     {
     }
   case PLUGIN_WEBFORM_SAVE:
     {
       success = true;
       break;
     }
   case PLUGIN_INIT:
     {
       break;
     }
   case PLUGIN_ONCE_A_SECOND:
     {
       amg.readPixels(pixels);
       float sum = 0;
       float mini = 80;
       float maxi = 0;
       float average = 0;
       for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++)
       {
         if ( pixels[i-1] < mini ) mini = pixels[i-1]; if ( pixels[i-1] > maxi ) maxi = pixels[i-1];
         sum += pixels[i-1];
       }
       average = sum/AMG88xx_PIXEL_ARRAY_SIZE;
       String log = F("AMG88xx : Temp.mini: ");
       log += mini;
       UserVar[event->BaseVarIndex + 0] = mini;
       addLog(LOG_LEVEL_INFO, log);
       log = F("AMG88xx : Temp.maxi: ");
       log += maxi;
       UserVar[event->BaseVarIndex + 1] = maxi;
       addLog(LOG_LEVEL_INFO, log);
       log = F("AMG88xx : Temp.avg: ");
       log += average;
       UserVar[event->BaseVarIndex + 2] = average;
       addLog(LOG_LEVEL_INFO, log);

       success = true;
       break;
     }
   }
    return success;
  }
#endif // USES_P116
