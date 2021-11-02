#ifdef PLUGIN_BUILD_DEV //at the very beginning

#include "_Plugin_Helper.h"
#ifdef USES_P121

// #######################################################################################################
// #################### Plugin 121 - Adafruit HMC5883L Driver (3-Axis Magnetometer)               ########
// #######################################################################################################

/*******************************************************************************
* Copyright 2021
* Written by Sven Briesen (adsorstuff@gmail.com)
* BSD license, all text above must be included in any redistribution
*
* Release notes:
* Adafruit_HMC5883_Unified Library v1.2.0 required (https://github.com/adafruit/Adafruit_HMC5883_Unified)
* Used P106 BME680 as starting point
/******************************************************************************/

# include <Adafruit_Sensor.h>
# include <Adafruit_HMC5883_U.h>


# define PLUGIN_121
# define PLUGIN_ID_121         121
# define PLUGIN_NAME_121       "Environment - HCM5883L"
# define PLUGIN_VALUENAME1_121 "x"
# define PLUGIN_VALUENAME2_121 "y"
# define PLUGIN_VALUENAME3_121 "z"


boolean Plugin_121(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;
  boolean initialized = false;

  Adafruit_HMC5883_Unified mag; // I2C

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_121;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
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
      string = F(PLUGIN_NAME_121);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_121));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_121));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_121));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex);
      getPluginTaskData(event->TaskIndex);

      mag = Adafruit_HMC5883_Unified(12345);

      if (!mag.begin())
        {
          addLog(LOG_LEVEL_ERROR, F("HCM5883L : Ooops, no HMC5883 detected ... Check your wiring!"));
          success = false;
          break;
        }

      sensor_t sensor;
      mag.getSensor(&sensor);

    
      String sensorStr = F("Sensor:       ");
      sensorStr += sensor.name;
      
      String driverStr = F("Driver Ver:   ");
      driverStr += sensor.version;
      
      String uniqueIdStr = F("Unique ID:    ");
      uniqueIdStr += sensor.sensor_id;
  
      String maxValStr = F("Max Value:    ");
      maxValStr += sensor.max_value;
  
      String minValStr =F("Min Value:    ");
      minValStr += sensor.min_value;
  
      String resolutionStr =F("Resolution:   ");
      resolutionStr += sensor.resolution;  
  
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
      addLog(LOG_LEVEL_DEBUG, sensorStr);
      addLog(LOG_LEVEL_DEBUG, driverStr);
      addLog(LOG_LEVEL_DEBUG, uniqueIdStr);
      addLog(LOG_LEVEL_DEBUG, maxValStr); 
      addLog(LOG_LEVEL_DEBUG, minValStr); 
      addLog(LOG_LEVEL_DEBUG, resolutionStr); 
      addLog(LOG_LEVEL_DEBUG, F("------------------------------------"));
      break;
    }
    case PLUGIN_READ:
    {
      P121_data_struct *P121_data =
        static_cast<P121_data_struct *>(getPluginTaskData(event->TaskIndex));


  
      sensors_event_t s_event; 
      if (!mag.getEvent(&s_event))
        addLog(LOG_LEVEL_ERROR, F("HCM5883L : Failed to perform reading!"));
        success = false;
        break;
      }
      
      String values = "HCM5883L : x: " + String(s_event.magnetic.x) + ", y: " + String(s_event.magnetic.y) + ", z: " + String(s_event.magnetic.z);
      addLog(LOG_LEVEL_DEBUG, values);

      UserVar[event->BaseVarIndex + 0] = s_event.magnetic.x;
      UserVar[event->BaseVarIndex + 1] = s_event.magnetic.y;
      UserVar[event->BaseVarIndex + 2] = s_event.magnetic.z;      

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P121
#endif //at the very end
