#include "../PluginStructs/P026_data_struct.h"

#ifdef USES_P026

# include "../DataStructs/ESPEasy_packed_raw_data.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Helpers/Memory.h"
# include "../Helpers/Hardware_temperature_sensor.h"
# ifdef ESP32
#  include "../Helpers/Hardware_device_info.h"

# endif // ifdef ESP32

# include "ESPEasy-Globals.h"

// Do not change assigned values as they are stored
// Shown selection and its order can be set in P026_value_option_indices
// These P026_VALUETYPE_xxx values should represent the index in the p026_valuenames array
# define P026_VALUETYPE_uptime       0
# define P026_VALUETYPE_freeheap     1
# define P026_VALUETYPE_rssi         2
# define P026_VALUETYPE_vcc          3
# define P026_VALUETYPE_load         4
# define P026_VALUETYPE_ip1          5
# define P026_VALUETYPE_ip2          6
# define P026_VALUETYPE_ip3          7
# define P026_VALUETYPE_ip4          8
# define P026_VALUETYPE_web          9
# define P026_VALUETYPE_freestack    10
# define P026_VALUETYPE_none         11
# define P026_VALUETYPE_txpwr        12
# define P026_VALUETYPE_free2ndheap  13
# define P026_VALUETYPE_internaltemp 14
# define P026_VALUETYPE_freepsram    15


const __FlashStringHelper* Plugin_026_valuename(uint8_t value_nr, bool displayString) {
  const __FlashStringHelper *p026_valuenames[] {
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
# ifdef USE_SECOND_HEAP
    F("Free 2nd Heap"), F("free2ndheap"),
# else // ifdef USE_SECOND_HEAP
    F(""), F(""), // Must keep the same indexes
# endif // ifdef USE_SECOND_HEAP
# if FEATURE_INTERNAL_TEMPERATURE
    F("Internal temperature (ESP32)"), F("internaltemp"),
# else // if FEATURE_INTERNAL_TEMPERATURE
    F(""), F(""), // Must keep the same indexes
# endif // if FEATURE_INTERNAL_TEMPERATURE

# if defined(ESP32) && defined(BOARD_HAS_PSRAM)
    F("Free PSRAM"), F("freepsram"),
# else // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
    F(""), F(""), // Must keep the same indexes
# endif // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
  };

  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(p026_valuenames);

  if (index < nrStrings) {
    return p026_valuenames[index];
  }
  return F("");
}

// List of options in the order how they will be shown in the plugin selector.
const int P026_value_option_indices[] = {
  P026_VALUETYPE_none, // Have the "none" option as first option

  P026_VALUETYPE_uptime,
  P026_VALUETYPE_load,
  P026_VALUETYPE_freeheap,
# if defined(ESP32) && defined(BOARD_HAS_PSRAM)
  P026_VALUETYPE_freepsram,
# endif // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
# ifdef USE_SECOND_HEAP
  P026_VALUETYPE_free2ndheap,
# endif // ifdef USE_SECOND_HEAP
  P026_VALUETYPE_freestack,
  P026_VALUETYPE_rssi,
  P026_VALUETYPE_txpwr,
# if FEATURE_ADC_VCC
  P026_VALUETYPE_vcc,
# endif // if FEATURE_ADC_VCC
# if FEATURE_INTERNAL_TEMPERATURE
  P026_VALUETYPE_internaltemp,
# endif // if FEATURE_INTERNAL_TEMPERATURE
  P026_VALUETYPE_ip1,
  P026_VALUETYPE_ip2,
  P026_VALUETYPE_ip3,
  P026_VALUETYPE_ip4,
  P026_VALUETYPE_web,
};


float P026_get_value(uint8_t type)
{
  float res{};

  switch (type)
  {
    case P026_VALUETYPE_uptime:   res = getUptimeMinutes(); break;
    case P026_VALUETYPE_freeheap: res = FreeMem(); break;
    case P026_VALUETYPE_rssi:     res = WiFi.RSSI(); break;
    case P026_VALUETYPE_vcc:
# if FEATURE_ADC_VCC
      res = vcc;
# else // if FEATURE_ADC_VCC
      res = -1.0f;
# endif // if FEATURE_ADC_VCC
      break;
    case P026_VALUETYPE_load: res = getCPUload(); break;
    case P026_VALUETYPE_ip1:
    case P026_VALUETYPE_ip2:
    case P026_VALUETYPE_ip3:
    case P026_VALUETYPE_ip4:
      res = NetworkLocalIP()[type - 5];
      break;
    case P026_VALUETYPE_web:
      res = timePassedSince(lastWeb) / 1000.0f;
      break; // respond in seconds
    case P026_VALUETYPE_freestack: res = getCurrentFreeStack(); break;
    case P026_VALUETYPE_txpwr:     res = WiFiEventData.wifi_TX_pwr; break;
# ifdef USE_SECOND_HEAP
    case P026_VALUETYPE_free2ndheap:
      res = FreeMem2ndHeap();
      break;
# endif // ifdef USE_SECOND_HEAP
# if FEATURE_INTERNAL_TEMPERATURE
    case P026_VALUETYPE_internaltemp:
      res = getInternalTemperature();
      break;
# endif // if FEATURE_INTERNAL_TEMPERATURE
# if defined(ESP32) && defined(BOARD_HAS_PSRAM)
    case P026_VALUETYPE_freepsram:

      if (UsePSRAM()) {
        res = ESP.getFreePsram();
      }
      break;
# endif // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
  }
  return res;
}

bool P026_data_struct::GetDeviceValueNames(struct EventStruct *event)
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
  return true;
}

bool P026_data_struct::WebformLoadOutputSelector(struct EventStruct *event)
{
  constexpr size_t NrOptions = NR_ELEMENTS(P026_value_option_indices);

  const __FlashStringHelper *options[NrOptions];

  for (uint8_t index = 0; index < NrOptions; ++index) {
    options[index] = Plugin_026_valuename(P026_value_option_indices[index], true);
  }

  const int valueCount = P026_NR_OUTPUT_VALUES;

  for (uint8_t i = 0; i < valueCount; ++i) {
    const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
    sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, NrOptions, options, P026_value_option_indices);
  }
  return true;
}

bool P026_data_struct::WebformSave(struct EventStruct *event)
{
  // Save output selector parameters.
  const int valueCount = P026_NR_OUTPUT_VALUES;

  for (uint8_t i = 0; i < valueCount; ++i) {
    const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
    const uint8_t choice       = PCONFIG(pconfigIndex);
    sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_026_valuename(choice, false));
  }
  return true;
}

bool P026_data_struct::Plugin_Read(struct EventStruct *event)
{
  const int valueCount = P026_NR_OUTPUT_VALUES;

  for (int i = 0; i < valueCount; ++i) {
    UserVar.setFloat(event->TaskIndex, i,  P026_get_value(PCONFIG(i)));
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
  return true;
}

# ifndef PLUGIN_BUILD_MINIMAL_OTA
bool P026_data_struct::Plugin_GetConfigValue(struct EventStruct *event, String& string)
{
  bool success     = false;
  const String cmd = parseString(string, 1);

  constexpr size_t P026_NR_OUTPUT_OPTIONS = NR_ELEMENTS(P026_value_option_indices);

  for (uint8_t option = 0; option < P026_NR_OUTPUT_OPTIONS; ++option) {
    if ((P026_value_option_indices[option] != P026_VALUETYPE_none) &&
        equals(cmd, Plugin_026_valuename(P026_value_option_indices[option], false))) {     // Use default valuename
      string  = floatToString(P026_get_value(P026_value_option_indices[option]), 2, true); // Trim trailing zeroes
      success = true;
      break;
    }
  }

  return success;
}

# endif // ifndef PLUGIN_BUILD_MINIMAL_OTA


# if FEATURE_PACKED_RAW_DATA
bool P026_data_struct::Plugin_GetPackedRawData(struct EventStruct *event, String& string)
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
  return true;
}

# endif // if FEATURE_PACKED_RAW_DATA


#endif  // ifdef USES_P026
