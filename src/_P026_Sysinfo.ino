#include "_Plugin_Helper.h"
#ifdef USES_P026

// #######################################################################################################
// #################################### Plugin 026: System Info ##########################################
// #######################################################################################################


# include "src/DataStructs/ESPEasy_packed_raw_data.h"
# include "src/ESPEasyCore/ESPEasyNetwork.h"
# include "src/Globals/ESPEasyWiFiEvent.h"
# include "src/Helpers/Memory.h"

# include "ESPEasy-Globals.h"

# define PLUGIN_026
# define PLUGIN_ID_026         26
# define PLUGIN_NAME_026       "Generic - System Info"

// place sensor type selector right after the output value settings
# define P026_QUERY1_CONFIG_POS  0
# define P026_SENSOR_TYPE_INDEX  (P026_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P026_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX)))

# define P026_NR_OUTPUT_OPTIONS  14

const __FlashStringHelper* Plugin_026_valuename(uint8_t value_nr, bool displayString) {
  const __FlashStringHelper* strings[] {
    F("Uptime")       , F("uptime"),
    F("Free RAM")     , F("freeheap"),
    F("Wifi RSSI")    , F("rssi"),
    F("Input VCC")    , F("vcc"),
    F("System load")  , F("load"),
    F("IP 1.Octet")   , F("ip1"),
    F("IP 2.Octet")   , F("ip2"),
    F("IP 3.Octet")   , F("ip3"),
    F("IP 4.Octet")   , F("ip4"),
    F("Web activity") , F("web"),
    F("Free Stack")   , F("freestack"),
    F("None")         , F(""),
    F("WiFi TX pwr")  , F("txpwr"),
    F("Free 2nd Heap"), F("free2ndheap")
  };
  const size_t index = (2* value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = sizeof(strings) / sizeof(strings[0]);
  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

boolean Plugin_026(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_026;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
      Device[deviceCount].PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_026);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P026_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_026_valuename(PCONFIG(pconfigIndex), false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
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
      const __FlashStringHelper *options[P026_NR_OUTPUT_OPTIONS];
      int indices[P026_NR_OUTPUT_OPTIONS];

      int index = 0;

      for (uint8_t option = 0; option < P026_NR_OUTPUT_OPTIONS; ++option) {
        if (option != 11) {
          options[index] = Plugin_026_valuename(option, true);
          indices[index] = option;
          ++index;
        }
      }

      // Work around to get the "none" at the end.
      options[index] = Plugin_026_valuename(11, true);
      indices[index] = 11;

      for (uint8_t i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P026_NR_OUTPUT_OPTIONS, options, indices);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // Save output selector parameters.
      for (uint8_t i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_026_valuename(choice, false));
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        UserVar[event->BaseVarIndex + i] = P026_get_value(PCONFIG(i));
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;

        if (log.reserve(7 * (P026_NR_OUTPUT_VALUES + 1)))
        {
          log += F("SYS  : ");

          for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
            if (i != 0) {
              log += ',';
            }
            log += formatUserVarNoCheck(event->TaskIndex, i);
          }
          addLogMove(LOG_LEVEL_INFO, log);
        }
      }
      success = true;
      break;
    }
# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // Matching JS code:
      // return decode(bytes,
      //  [header, uint24, uint24, int8, vcc, pct_8, uint8, uint8, uint8, uint8, uint24, uint16],
      //  ['header', 'uptime', 'freeheap', 'rssi', 'vcc', 'load', 'ip1', 'ip2', 'ip3', 'ip4', 'web', 'freestack']);
      uint8_t index = 0;
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // uptime
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // freeheap
      string     += LoRa_addFloat(P026_get_value(index++), PackedData_int8);  // rssi
      string     += LoRa_addFloat(P026_get_value(index++), PackedData_vcc);   // vcc
      string     += LoRa_addFloat(P026_get_value(index++), PackedData_pct_8); // load
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip1
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip2
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip3
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip4
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // web
      string     += LoRa_addInt(P026_get_value(index++), PackedData_uint16);  // freestack
      event->Par1 = index;                                                    // valuecount
      success     = true;
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA
  }
  return success;
}

float P026_get_value(uint8_t type)
{
  switch (type)
  {
    case 0: return getUptimeMinutes();
    case 1: return FreeMem();
    case 2: return WiFi.RSSI();
    case 3:
# if FEATURE_ADC_VCC
      return vcc;
# else // if FEATURE_ADC_VCC
      return -1.0f;
# endif // if FEATURE_ADC_VCC
    case 4: return getCPUload();
    case 5:
    case 6:
    case 7:
    case 8:
      return NetworkLocalIP()[type - 5];
    case 9:  return timePassedSince(lastWeb) / 1000.0f; // respond in seconds
    case 10: return getCurrentFreeStack();
    case 12: return WiFiEventData.wifi_TX_pwr;
    case 13:
      # ifdef USE_SECOND_HEAP
      return FreeMem2ndHeap();
      # else // ifdef USE_SECOND_HEAP
      break;
      # endif // ifdef USE_SECOND_HEAP
  }
  return 0.0f;
}

#endif // USES_P026
