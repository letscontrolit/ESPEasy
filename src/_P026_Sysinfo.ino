#include "_Plugin_Helper.h"
#ifdef USES_P026

// #######################################################################################################
// #################################### Plugin 026: System Info ##########################################
// #######################################################################################################

/** Changelog:
 * 2023-09-24 tonhuisman: Add support for getting all values via Get Config option [<taskname>#<valuename>] where <valuename> is the default
 *                        name as set for an output value. None is ignored. Not available in MINIMAL_OTA builds.
 *                        Move all includes to P026_data_struct.h
 * 2023-09-23 tonhuisman: Add Internal temperature option for ESP32
 *                        Format source using Uncrustify
 *                        Move #if check to P026_data_struct.h as Arduino compiler doesn't support that :(
 *                        Move other defines to P026_data_struct.h
 * 2023-09-23 tonhuisman: Start changelog
 */

# define PLUGIN_026
# define PLUGIN_ID_026         26
# define PLUGIN_NAME_026       "Generic - System Info"

# include "src/PluginStructs/P026_data_struct.h" // Arduino doesn't do #if in .ino sources :(



boolean Plugin_026(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_026;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.FormulaOption  = true;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_026);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      success = P026_data_struct::GetDeviceValueNames(event);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P026_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX));
      event->idx        = P026_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = 0;    // "Uptime"

      for (uint8_t i = 1; i < VARS_PER_TASK; ++i) {
        PCONFIG(i) = 11; // "None"
      }
      PCONFIG(P026_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      success                         = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      success = P026_data_struct::WebformLoadOutputSelector(event);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P026_data_struct::WebformSave(event);
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      success = P026_data_struct::Plugin_Read(event);
      break;
    }
    # ifndef PLUGIN_BUILD_MINIMAL_OTA
    case PLUGIN_GET_CONFIG_VALUE:
    {
      success = P026_data_struct::Plugin_GetConfigValue(event, string);
      break;
    }
    # endif // ifndef PLUGIN_BUILD_MINIMAL_OTA
# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      success = P026_data_struct::Plugin_GetPackedRawData(event, string);
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA
  }
  return success;
}


#endif // USES_P026
