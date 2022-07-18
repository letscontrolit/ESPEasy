
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

#include "src/PluginStructs/P121_data_struct.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <math.h>

#define PLUGIN_121
#define PLUGIN_ID_121 121
#define PLUGIN_NAME_121 "Position - HCM5883L"
#define PLUGIN_VALUENAME1_121 "x"
#define PLUGIN_VALUENAME2_121 "y"
#define PLUGIN_VALUENAME3_121 "z"
#define PLUGIN_VALUENAME4_121 "dir"

boolean Plugin_121(uint8_t function, struct EventStruct *event, String &string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_121;
    Device[deviceCount].Type = DEVICE_TYPE_I2C;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    Device[deviceCount].PluginStats        = true;
    success = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    string = F(PLUGIN_NAME_121);
    success = true;
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_121));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_121));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_121));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_121));
    success = true;
    break;
  }
  case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
  {
    success = true;
    break;
  }
  case PLUGIN_I2C_HAS_ADDRESS:
  {
    success = (event->Par1 == 0x1E);
    break;
  }
  case PLUGIN_WEBFORM_LOAD:
  {
    addFormFloatNumberBox(F("DeclinationAngle"), F("plugin_121_HMC5883L_decl"), PCONFIG_FLOAT(0), -180.0f,180.0f ,2, 0.01f);
    PCONFIG_FLOAT(1) = PCONFIG_FLOAT(0) * M_PI / 180.0; // convert from degree to radian
    addUnit(F("degree"));
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_121_HMC5883L_decl"));
    success = true;
    break;
  }

  case PLUGIN_INIT:
  {
    initPluginTaskData(event->TaskIndex, new (std::nothrow) P121_data_struct());
    P121_data_struct *P121_data =
        static_cast<P121_data_struct *>(getPluginTaskData(event->TaskIndex));

    if (nullptr != P121_data)
    {
      P121_data->begin(event->TaskIndex);
      success = P121_data->initialized;
    }
    break;
  }
  case PLUGIN_READ:
  {
    P121_data_struct *P121_data =
        static_cast<P121_data_struct *>(getPluginTaskData(event->TaskIndex));

    if (nullptr != P121_data)
    {
      P121_data->begin(event->TaskIndex);

      if (!P121_data->initialized)
      {
        addLog(LOG_LEVEL_ERROR, F("Could not initialize HCM5883L"));
        break;
      }
      sensors_event_t s_event;
      P121_data->mag.getEvent(&s_event);
      UserVar[event->BaseVarIndex + 0] = s_event.magnetic.x;
      UserVar[event->BaseVarIndex + 1] = s_event.magnetic.y;
      UserVar[event->BaseVarIndex + 2] = s_event.magnetic.z;

      float heading = atan2(s_event.magnetic.y, s_event.magnetic.x);

      const float decl = PCONFIG_FLOAT(1);
      
      if (decl != 0)
      {
        heading += decl;
      }

      if (heading < 0)
        heading += 2*PI;

      if (heading > 2*PI)
        heading -= 2*PI;

      float degreeHeading = heading * 180/M_PI;
            UserVar[event->BaseVarIndex + 3] = degreeHeading;


    }

    break;
  }
  }
  return success;
}
#endif // ifdef USES_P121