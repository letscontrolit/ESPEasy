#include "../Helpers/_Plugin_SensorTypeHelper.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/DeviceStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Device.h"
#include "../Globals/Plugins.h"

#include "../WebServer/Markup.h"



void sensorTypeHelper_webformLoad_allTypes(struct EventStruct *event, uint8_t pconfigIndex)
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
#endif
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
#endif
    static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_STRING)
  };
  constexpr int optionCount = NR_ELEMENTS(optionValues);

  sensorTypeHelper_webformLoad(event, pconfigIndex, optionCount, optionValues);
}

void sensorTypeHelper_webformLoad_simple(struct EventStruct *event, uint8_t pconfigIndex)
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

void sensorTypeHelper_webformLoad(struct EventStruct *event, uint8_t pconfigIndex, int optionCount, const uint8_t options[])
{
  addFormSubHeader(F("Output Configuration"));

  if (pconfigIndex >= PLUGIN_CONFIGVAR_MAX) {
    return;
  }
  Sensor_VType choice             = static_cast<Sensor_VType>(PCONFIG(pconfigIndex));
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) {
    choice                = Sensor_VType::SENSOR_TYPE_NONE;
    PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
  } else if (getValueCountFromSensorType(choice) != getValueCountForTask(event->TaskIndex)) {
    // Invalid value
    checkDeviceVTypeForTask(event);
    choice                = event->sensorType;
    PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
  }

  const __FlashStringHelper *outputTypeLabel = F("Output Data Type");

  if (Device[DeviceIndex].OutputDataType ==  Output_Data_type_t::Simple) {
    if (!isSimpleOutputDataType(event->sensorType))
    {
      choice                = Device[DeviceIndex].VType;
      PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
    }
    outputTypeLabel = F("Number Output Values");
  }
  addRowLabel(outputTypeLabel);
  addSelector_Head(PCONFIG_LABEL(pconfigIndex));

  for (uint8_t x = 0; x < optionCount; x++)
  {
    String name = getSensorTypeLabel(static_cast<Sensor_VType>(options[x]));
    addSelector_Item(name,
                     options[x],
                     choice == static_cast<Sensor_VType>(options[x]));
  }
  addSelector_Foot();
  {
    String note;
    note  = F("Changing '");
    note += outputTypeLabel;
    note += F("' may affect behavior of some controllers (e.g. Domoticz)");
    addFormNote(note);
  }
  String dummy;

  PluginCall(PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR, event, dummy);
}

void sensorTypeHelper_saveOutputSelector(struct EventStruct *event, uint8_t pconfigIndex, uint8_t valueIndex, const String& defaultValueName)
{
  const bool isDefault = defaultValueName.equals(ExtraTaskSettings.TaskDeviceValueNames[valueIndex]);
  if (isDefault) {
    ExtraTaskSettings.clearTaskDeviceValueName(valueIndex);
  }
  ExtraTaskSettings.isDefaultTaskVarName(valueIndex, isDefault);
  pconfig_webformSave(event, pconfigIndex);
}

void pconfig_webformSave(struct EventStruct *event, uint8_t pconfigIndex)
{
  PCONFIG(pconfigIndex) = getFormItemInt(PCONFIG_LABEL(pconfigIndex), PCONFIG(pconfigIndex));
}

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const __FlashStringHelper *options[], const int indices[])
{
  addFormSelector(
    concat(F("Value "), valuenr + 1),
    PCONFIG_LABEL(pconfigIndex),
    optionCount,
    options,
    indices,
    PCONFIG(pconfigIndex));
}

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const String options[], const int indices[])
{
  addFormSelector(
    concat(F("Value "), valuenr + 1),
    PCONFIG_LABEL(pconfigIndex),
    optionCount,
    options,
    indices,
    PCONFIG(pconfigIndex));
}
