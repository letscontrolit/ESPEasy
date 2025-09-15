#include "../Helpers/_Plugin_SensorTypeHelper.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/DeviceStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Device.h"
#include "../Globals/Plugins.h"

#include "../WebServer/Markup.h"


void sensorTypeHelper_webformLoad_allTypes(struct EventStruct *event, int pconfigIndex)
{
  const uint8_t optionValues[] {
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TRIPLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_HUM),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_BARO),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SWITCH),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DIMMER),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_WIND),
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT32_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT32_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT32_TRIPLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT32_QUAD),
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_ULONG),
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_UINT32_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_UINT32_QUAD),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT64_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_INT64_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_UINT64_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_UINT64_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL),
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_STRING)
  };
  constexpr int optionCount = NR_ELEMENTS(optionValues);

  sensorTypeHelper_webformLoad(event, pconfigIndex, optionCount, optionValues);
}

void sensorTypeHelper_webformLoad_simple(struct EventStruct *event, int pconfigIndex)
{
  const uint8_t optionValues[] {
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DUAL),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TRIPLE),
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD)
  };
  constexpr int optionCount = NR_ELEMENTS(optionValues);

  sensorTypeHelper_webformLoad(event, pconfigIndex, optionCount, optionValues);
}

void sensorTypeHelper_Selector(const String& id, int optionCount, const uint8_t options[], Sensor_VType choice) {
  addSelector_Head(id);

  for (uint8_t x = 0; x < optionCount; x++)
  {
    const Sensor_VType optionVType = static_cast<Sensor_VType>(options[x]);
    addSelector_Item(getSensorTypeLabel(optionVType),
                     options[x],
                     choice == optionVType);
  }
  addSelector_Foot();
}

#if FEATURE_CUSTOM_TASKVAR_VTYPE
const char value_type_categories[] PROGMEM =
  "Basic|"       // 1024
  "Environment|" // 1025
  "Dust/Gases|"  // 1026
  "Energy|"      // 1027
  "Time|"        // 1028
  "Size|"        // 1029
  "Light|"       // 1030
  "Other|"       // 1031
;

String toValueTypeCategory(const uint32_t valueType) {
  char   tmp[12]{};
  String result;

  result = GetTextIndexed(tmp, sizeof(tmp), valueType, value_type_categories);

  return result;
}

/* *INDENT-OFF* */

const uint16_t value_type_map[] PROGMEM = {
  0,                                                                                          // None
  1024, 1,   5,   6,   7,                                                                     // Basic
  1025, 2,   3,   4,   8,   101, 102, 122, 21,  131, 143, 144, 151, 152,                      // Environment
  1026, 106, 107, 108, 110, 146, 121, 154, 128, 129, 148, 149, 150, 153,                      // Gases
  1027, 116, 117, 118, 119, 120, 127, 139, 141, 142, 147,                                     // Energy
  1028, 132, 133, 134,                                                                        // Time
  1029, 100, 104, 105, 109, 111, 115, 135, 136, 137, 138, 155, 156, 157, 158,                 // Size
  1030, 103, 112, 113, 114, 123, 124, 125, 126,                                               // Light
  1031, 10,  130, 11,  22,  20,  31,  32,  33,  40,  41, 42, 43, 50, 51, 60, 61, 70, 71, 140, // Other
        145,                                                                                  // Other
};

/* *INDENT-ON* */

void sensorTypeCategoriesHelper_Selector(const String& id,
                                         int           optionCount,
                                         const uint8_t options[],
                                         Sensor_VType  choice) {
  constexpr uint16_t asize = NR_ELEMENTS(value_type_map);
  bool firstGrp            = true;
  bool closeGrp            = false;
  const uint8_t iChoice    = static_cast<uint8_t>(choice);
  const std::vector<uint8_t> vOptions(options, options + optionCount);

  do_addSelector_Head(id, F("xwide"), EMPTY_STRING, false
                      # if FEATURE_TOOLTIPS
                      , F("¹: Used for MQTT AutoDiscovery")
                      # endif // if FEATURE_TOOLTIPS
                      );

  for (uint16_t idx = 0; idx < asize; ++idx) {
    const uint16_t vtIdx = pgm_read_word_near(&value_type_map[idx]);

    if (vtIdx < 1024) {
      const uint8_t vtIdx8 = vtIdx;

      if (std::find(vOptions.begin(), vOptions.end(), vtIdx8) != vOptions.end()) {
        const Sensor_VType vt = static_cast<Sensor_VType>(vtIdx8);
        addSelector_Item(
          concat(getSensorTypeLabel(vt), isMQTTDiscoverySensorType(vt) ? F("¹") : EMPTY_STRING),
          vtIdx,
          iChoice == vtIdx8);
      }
    }

    if (vtIdx >= 1024) {
      uint16_t vtIdx2   = 0;
      bool     addGroup = false; // Check if group should be included

      for (uint8_t idx2 = idx + 1; !addGroup && (vtIdx2 < 1024) && (idx2 < asize); ++idx2) {
        vtIdx2 = pgm_read_word_near(&value_type_map[idx2]);

        if (vtIdx2 < 1024) {
          const uint8_t vtIdx8 = vtIdx2;

          if (std::find(vOptions.begin(), vOptions.end(), vtIdx8) != vOptions.end()) {
            addGroup = true;
          }
        }
      }

      if (addGroup) {
        if (!firstGrp) {
          addSelector_OptGroupFoot();
          closeGrp = true;
        }
        addSelector_OptGroup(toValueTypeCategory(vtIdx - 1024));
        firstGrp = false;
        closeGrp = false;
      }
    }

    if ((idx & 0x07) == 0) { delay(0); }
  }

  if (!firstGrp && !closeGrp) {
    addSelector_OptGroupFoot();
  }
  addSelector_Foot();
}

#endif // if FEATURE_CUSTOM_TASKVAR_VTYPE

void sensorTypeHelper_webformLoad(struct EventStruct *event, int pconfigIndex, int optionCount, const uint8_t options[])
{
  sensorTypeHelper_webformLoad(event, pconfigIndex, optionCount, options, true, 0);
}

void sensorTypeHelper_webformLoad(struct EventStruct *event,
                                  int                 pconfigIndex,
                                  int                 optionCount,
                                  const uint8_t       options[],
                                  bool                showSubHeader,
                                  int                 valueIndex)
{
  if (showSubHeader) {
    addFormSubHeader(F("Output Configuration"));
  }

  if ((pconfigIndex < 0) || (pconfigIndex >= PLUGIN_CONFIGVAR_MAX)) {
    return;
  }
  Sensor_VType choice             = static_cast<Sensor_VType>(PCONFIG(pconfigIndex));
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

  if (showSubHeader) {
    if (!validDeviceIndex(DeviceIndex)) {
      // FIXME TD-er: Should we even continue here?
      choice                = Sensor_VType::SENSOR_TYPE_NONE;
      PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
    } else if (getValueCountFromSensorType(choice) != getValueCountForTask(event->TaskIndex)) {
      // Invalid value
      if (checkDeviceVTypeForTask(event) >= 0) {
        choice                = event->sensorType;
        PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
      }
    }
  }

  const __FlashStringHelper *outputTypeLabel = F("Output Data Type");

  if (validDeviceIndex(DeviceIndex) && (Device[DeviceIndex].OutputDataType ==  Output_Data_type_t::Simple)) {
    if (!isSimpleOutputDataType(event->sensorType))
    {
      choice                = Device[DeviceIndex].VType;
      PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
    }
    outputTypeLabel = F("Number Output Values");
  }

  if (showSubHeader) {
    addRowLabel(outputTypeLabel);
  } else if (valueIndex >= 0) {
    addRowLabel(strformat(F("Value %d type"), valueIndex));
  }

  #if FEATURE_CUSTOM_TASKVAR_VTYPE
  if (optionCount > 4) { // Don't handle the 'Simple' selector categorized
    sensorTypeCategoriesHelper_Selector(sensorTypeHelper_webformID(pconfigIndex), optionCount, options, choice);
  } else
  #endif // if FEATURE_CUSTOM_TASKVAR_VTYPE
  {
    sensorTypeHelper_Selector(sensorTypeHelper_webformID(pconfigIndex), optionCount, options, choice);
  }

  if (showSubHeader) {
    String note;
    note  = F("Changing '");
    note += outputTypeLabel;
    note += F("' may affect behavior of some controllers (e.g. Domoticz)");
    addFormNote(note);
  }
  String dummy;

  PluginCall(PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR, event, dummy);
}

void sensorTypeHelper_saveOutputSelector(struct EventStruct *event, int pconfigIndex, uint8_t valueIndex, const String& defaultValueName)
{
  const bool isDefault = defaultValueName.equals(ExtraTaskSettings.TaskDeviceValueNames[valueIndex]);

  if (isDefault) {
    ExtraTaskSettings.clearTaskDeviceValueName(valueIndex);
  }
  ExtraTaskSettings.isDefaultTaskVarName(valueIndex, isDefault);
  pconfig_webformSave(event, pconfigIndex);
}

void pconfig_webformSave(struct EventStruct *event, int pconfigIndex)
{
  if ((pconfigIndex < 0) || (pconfigIndex >= PLUGIN_CONFIGVAR_MAX)) {
    return;
  }

  PCONFIG(pconfigIndex) = getFormItemInt(sensorTypeHelper_webformID(pconfigIndex), PCONFIG(pconfigIndex));
}

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, int pconfigIndex, uint8_t valuenr,
  int optionCount, const __FlashStringHelper *options[], const int indices[])
{
  if ((pconfigIndex < 0) || (pconfigIndex >= PLUGIN_CONFIGVAR_MAX)) {
    return;
  }
  const FormSelectorOptions selector(
    optionCount,
    options,
    indices);
  selector.addFormSelector(
    concat(F("Value "), valuenr + 1),
    sensorTypeHelper_webformID(pconfigIndex),
    PCONFIG(pconfigIndex));
}

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, int pconfigIndex, uint8_t valuenr,
  int optionCount, const String options[], const int indices[])
{
  if ((pconfigIndex < 0) || (pconfigIndex >= PLUGIN_CONFIGVAR_MAX)) {
    return;
  }
  const FormSelectorOptions selector(
    optionCount,
    options,
    indices);
  selector.addFormSelector(
    concat(F("Value "), valuenr + 1),
    sensorTypeHelper_webformID(pconfigIndex),
    PCONFIG(pconfigIndex));
}

String sensorTypeHelper_webformID(int pconfigIndex)
{
  if ((pconfigIndex >= 0) && (pconfigIndex < PLUGIN_CONFIGVAR_MAX)) {
    return concat(F("pconfigIndex_"), pconfigIndex);
  }
  return F("error");
}
