
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
/** Changelog:
 * 2022-11-06 tonhuisman: Fix compilation issue with older ESP8266 toolchain, reduce some strings, uncrustify sources,
 *                        minor code improvements.
 *                        Adafruit_HMC5883_Unified: Fix waiting indefinitely for a connected sensor.
 */

# include "src/PluginStructs/P121_data_struct.h"

# define PLUGIN_121
# define PLUGIN_ID_121          121
# define PLUGIN_NAME_121        "Position - HMC5883L"
# define PLUGIN_VALUENAME1_121  "x"
# define PLUGIN_VALUENAME2_121  "y"
# define PLUGIN_VALUENAME3_121  "z"
# define PLUGIN_VALUENAME4_121  "dir"

boolean Plugin_121(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_121;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      success            = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_121);
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

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x1E;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormFloatNumberBox(F("Declination Angle"), F("pdecl"), PCONFIG_FLOAT(0), -180.0f, 180.0f, 2, 0.01f);
      # define M_PI_180 0.01745329251994329577f       // M_PI / 180.0f
      PCONFIG_FLOAT(1) = PCONFIG_FLOAT(0) * M_PI_180; // M_PI / 180.0f; // convert from degree to radian
      addUnit(F("degree"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG_FLOAT(0) = getFormItemFloat(F("pdecl"));
      success          = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P121_data_struct());
      P121_data_struct *P121_data = static_cast<P121_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P121_data) && P121_data->begin(event->TaskIndex);
      break;
    }

    case PLUGIN_READ:
    {
      P121_data_struct *P121_data = static_cast<P121_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P121_data) {
        if (!P121_data->begin(event->TaskIndex)) {
          addLog(LOG_LEVEL_ERROR, F("Could not initialize HCM5883L"));
          break;
        }
        sensors_event_t s_event;
        P121_data->mag.getEvent(&s_event);

        UserVar.setFloat(event->TaskIndex, 0, s_event.magnetic.x);
        UserVar.setFloat(event->TaskIndex, 1, s_event.magnetic.y);
        UserVar.setFloat(event->TaskIndex, 2, s_event.magnetic.z);

        double heading = atan2(s_event.magnetic.y, s_event.magnetic.x);

        const double decl = PCONFIG_FLOAT(1);

        if (!essentiallyZero(decl)) {
          heading += decl;
        }

        if (definitelyLessThan(heading, 0)) {
          heading += TWO_PI;
        } else
        if (definitelyGreaterThan(heading, TWO_PI)) {
          heading -= TWO_PI;
        }

        UserVar.setFloat(event->TaskIndex, 3, heading * M_PI_180);

        success = true; // Assume we want to send out values to controllers
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P121
