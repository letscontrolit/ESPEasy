#include "_Plugin_Helper.h"
#ifdef USES_P033

// #######################################################################################################
// #################################### Plugin 033: Dummy ################################################
// #######################################################################################################

/** Changelog:
 * 2025-06-14 tonhuisman: Add support for Custom Value Type per task value
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported yet for Dummy Device)
 */

# define PLUGIN_033
# define PLUGIN_ID_033         33
# define PLUGIN_NAME_033       "Generic - Dummy Device"
# define PLUGIN_VALUENAME1_033 "Dummy"
boolean Plugin_033(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_033;
      dev.Type           = DEVICE_TYPE_DUMMY;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.DecimalsOnly   = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.OutputDataType = Output_Data_type_t::All;
      dev.PluginStats    = true;
      dev.CustomVTypeVar = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_033);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // FIXME TD-er: Copy names as done in P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_033));
      const Sensor_VType sensorType = static_cast<Sensor_VType>(PCONFIG(0));

      if (isIntegerOutputDataType(sensorType)) {
        for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(0)));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(0));
      event->idx        = 0;
      success           = true;
      break;
    }

    # if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      #  if FEATURE_CUSTOM_TASKVAR_VTYPE

      for (uint8_t i = 0; i < event->Par5; ++i) {
        event->ParN[i] = ExtraTaskSettings.getTaskVarCustomVType(i);  // Custom/User selection
      }
      #  else // if FEATURE_CUSTOM_TASKVAR_VTYPE
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      #  endif // if FEATURE_CUSTOM_TASKVAR_VTYPE
      success = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(0));
      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        const uint8_t valueCount =
          getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(0)));

        for (uint8_t x = 0; x < valueCount; ++x)
        {
          addLog(LOG_LEVEL_INFO,
                 strformat(F("Dummy: value %d: %s"), x + 1, formatUserVarNoCheck(event, x).c_str()));
        }
      }
      # endif // ifndef LIMIT_BUILD_SIZE
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P033
