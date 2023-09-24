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

const __FlashStringHelper* Plugin_026_valuename(uint8_t value_nr, bool displayString) {
  const __FlashStringHelper *strings[] {
    F("Uptime"), F("uptime"),
    F("Free RAM"), F("freeheap"),
    F("Wifi RSSI"), F("rssi"),
    F("Input VCC"), F("vcc"),
    F("System load"), F("load"),
    F("IP 1.Octet"), F("ip1"),
    F("IP 2.Octet"), F("ip2"),
    F("IP 3.Octet"), F("ip3"),
    F("IP 4.Octet"), F("ip4"),
    F("Web activity"), F("web"),
    F("Free Stack"), F("freestack"),
    F("None"), F(""),
    F("WiFi TX pwr"), F("txpwr"),
    F("Free 2nd Heap"), F("free2ndheap"),
    # if FEATURE_INTERNAL_TEMPERATURE
    F("Internal temperature (ESP32)"), F("internaltemp"),
    # endif // if FEATURE_INTERNAL_TEMPERATURE
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

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
      const int valueCount = P026_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_026_valuename(PCONFIG(pconfigIndex), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
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

      const int valueCount = P026_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < valueCount; ++i) {
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
      const int valueCount = P026_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < valueCount; ++i) {
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
      const int valueCount = P026_NR_OUTPUT_VALUES;

      for (int i = 0; i < valueCount; ++i) {
        UserVar[event->BaseVarIndex + i] = P026_get_value(PCONFIG(i));
      }
      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;

        if (log.reserve(7 * (valueCount + 1)))
        {
          log += F("SYS  : ");

          for (int i = 0; i < valueCount; ++i) {
            if (i != 0) {
              log += ',';
            }
            log += formatUserVarNoCheck(event->TaskIndex, i);
          }
          addLogMove(LOG_LEVEL_INFO, log);
        }
      }
      # endif // ifndef LIMIT_BUILD_SIZE
      success = true;
      break;
    }
    # ifndef PLUGIN_BUILD_MINIMAL_OTA
    case PLUGIN_GET_CONFIG_VALUE:
    {
      const String cmd = parseString(string, 1);

      for (uint8_t option = 0; option < P026_NR_OUTPUT_OPTIONS; ++option) {
        if ((option != 11) && equals(cmd, Plugin_026_valuename(option, false))) { // Use default valuename
          string  = floatToString(P026_get_value(option), 2, true);               // Trim trailing zeroes
          success = true;
          break;
        }
      }
      break;
    }
    # endif // ifndef PLUGIN_BUILD_MINIMAL_OTA
# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // Matching JS code:
      // return decode(bytes,
      //  [header, uint24, uint24, int8, vcc, pct_8, uint8, uint8, uint8, uint8, uint24, uint16],
      //  ['header', 'uptime', 'freeheap', 'rssi', 'vcc', 'load', 'ip1', 'ip2', 'ip3', 'ip4', 'web', 'freestack']);
      // on ESP32 you can add 'internaltemperature' of type int16 (1e2) to the list (disabled for now, so not available)
      uint8_t index = 0;
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // uptime
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // freeheap
      string += LoRa_addFloat(P026_get_value(index++), PackedData_int8);  // rssi
      string += LoRa_addFloat(P026_get_value(index++), PackedData_vcc);   // vcc
      string += LoRa_addFloat(P026_get_value(index++), PackedData_pct_8); // load
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip1
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip2
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip3
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip4
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // web
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint16);  // freestack
      // #  if FEATURE_INTERNAL_TEMPERATURE
      // string += LoRa_addInt(P026_get_value(index++) * 100.0f, PackedData_int16_1e2); // internal temperature in 0.01 degrees
      // #  endif // if FEATURE_INTERNAL_TEMPERATURE
      event->Par1 = index; // valuecount
      success     = true;
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA
  }
  return success;
}

float P026_get_value(uint8_t type)
{
  float res{};

  switch (type)
  {
    case 0: res = getUptimeMinutes(); break;
    case 1: res = FreeMem(); break;
    case 2: res = WiFi.RSSI(); break;
    case 3:
# if FEATURE_ADC_VCC
      res = vcc;
# else // if FEATURE_ADC_VCC
      res = -1.0f;
# endif // if FEATURE_ADC_VCC
      break;
    case 4: res = getCPUload(); break;
    case 5:
    case 6:
    case 7:
    case 8:
      res        = NetworkLocalIP()[type - 5]; break;
    case 9:  res = timePassedSince(lastWeb) / 1000.0f; break; // respond in seconds
    case 10: res = getCurrentFreeStack(); break;
    case 12: res = WiFiEventData.wifi_TX_pwr; break;
    case 13:
      # ifdef USE_SECOND_HEAP
      res = FreeMem2ndHeap();
      # endif // ifdef USE_SECOND_HEAP
      break;
    case 14:
      # if FEATURE_INTERNAL_TEMPERATURE
      res = getInternalTemperature();
      # endif // if FEATURE_INTERNAL_TEMPERATURE
      break;
  }
  return res;
}

#endif // USES_P026
