#include "../Helpers/_Plugin_SensorTypeHelper.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/DeviceStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Device.h"
#include "../Globals/Plugins.h"

#include "../WebServer/Markup.h"

/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
uint8_t getValueCountFromSensorType(Sensor_VType sensorType)
{
  switch (sensorType)
  {
    case Sensor_VType::SENSOR_TYPE_NONE:
      return 0;
    case Sensor_VType::SENSOR_TYPE_SINGLE: // single value sensor, used for Dallas, BH1750, etc
    case Sensor_VType::SENSOR_TYPE_SWITCH:
    case Sensor_VType::SENSOR_TYPE_DIMMER:
      return 1;
    case Sensor_VType::SENSOR_TYPE_LONG: // single LONG value, stored in two floats (rfid tags)
      return 1;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
    case Sensor_VType::SENSOR_TYPE_DUAL:
      return 2;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO: // Values 1 and 3 will contain data.
    case Sensor_VType::SENSOR_TYPE_TRIPLE:
    case Sensor_VType::SENSOR_TYPE_WIND:
      return 3;
    case Sensor_VType::SENSOR_TYPE_QUAD:
      return 4;
    case Sensor_VType::SENSOR_TYPE_STRING:  // String type data stored in the event->String2
      return 1;
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;
  }
  addLog(LOG_LEVEL_ERROR, F("getValueCountFromSensorType: Unknown sensortype"));
  return 0;
}

const __FlashStringHelper * getSensorTypeLabel(Sensor_VType sensorType) {
  switch (sensorType) {
    case Sensor_VType::SENSOR_TYPE_SINGLE:           return F("Single");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:         return F("Temp / Hum");
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:        return F("Temp / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:  return F("Temp / - / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:    return F("Temp / Hum / Baro");
    case Sensor_VType::SENSOR_TYPE_DUAL:             return F("Dual");
    case Sensor_VType::SENSOR_TYPE_TRIPLE:           return F("Triple");
    case Sensor_VType::SENSOR_TYPE_QUAD:             return F("Quad");
    case Sensor_VType::SENSOR_TYPE_SWITCH:           return F("Switch");
    case Sensor_VType::SENSOR_TYPE_DIMMER:           return F("Dimmer");
    case Sensor_VType::SENSOR_TYPE_LONG:             return F("Long");
    case Sensor_VType::SENSOR_TYPE_WIND:             return F("Wind");
    case Sensor_VType::SENSOR_TYPE_STRING:           return F("String");
    case Sensor_VType::SENSOR_TYPE_NONE:             return F("None");
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;
  }
  return F("");
}

void sensorTypeHelper_webformLoad_allTypes(struct EventStruct *event, uint8_t pconfigIndex)
{
  uint8_t optionValues[12];

  optionValues[0]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
  optionValues[1]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_HUM);
  optionValues[2]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_BARO);
  optionValues[3]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO);
  optionValues[4]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DUAL);
  optionValues[5]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TRIPLE);
  optionValues[6]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
  optionValues[7]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SWITCH);
  optionValues[8]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DIMMER);
  optionValues[9]  = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_LONG);
  optionValues[10] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_WIND);
  optionValues[11] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_STRING);
  sensorTypeHelper_webformLoad(event, pconfigIndex, 11, optionValues);
}

void sensorTypeHelper_webformLoad_simple(struct EventStruct *event, uint8_t pconfigIndex)
{
  uint8_t optionValues[4];
  optionValues[0] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
  optionValues[1] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DUAL);
  optionValues[2] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_TRIPLE);
  optionValues[3] = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
  sensorTypeHelper_webformLoad(event, pconfigIndex, 4, optionValues);
}

void sensorTypeHelper_webformLoad(struct EventStruct *event, uint8_t pconfigIndex, int optionCount, const uint8_t options[])
{
  addFormSubHeader(F("Output Configuration"));
  if (pconfigIndex >= PLUGIN_CONFIGVAR_MAX) {
    return;
  }
  Sensor_VType choice      = static_cast<Sensor_VType>(PCONFIG(pconfigIndex));
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);
  if (!validDeviceIndex(DeviceIndex)) {
    choice = Sensor_VType::SENSOR_TYPE_NONE;
    PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
  } else if (getValueCountFromSensorType(choice) != getValueCountForTask(event->TaskIndex)) {
    // Invalid value
    checkDeviceVTypeForTask(event);
    choice                = event->sensorType;
    PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
  }
  String outputTypeLabel = F("Output Data Type");
  if (Device[DeviceIndex].OutputDataType ==  Output_Data_type_t::Simple) {
    switch(event->sensorType) {
      case Sensor_VType::SENSOR_TYPE_SINGLE:
      case Sensor_VType::SENSOR_TYPE_DUAL:
      case Sensor_VType::SENSOR_TYPE_TRIPLE:
      case Sensor_VType::SENSOR_TYPE_QUAD:
        // These are valid
        break;
      default:
      {
        choice = Device[DeviceIndex].VType;
        PCONFIG(pconfigIndex) = static_cast<uint8_t>(choice);
        break;
      }
    }
    outputTypeLabel = F("Number Output Values");
  }
  addRowLabel(outputTypeLabel);
  addSelector_Head(PCONFIG_LABEL(pconfigIndex));

  for (uint8_t x = 0; x < optionCount; x++)
  {
    String name     = getSensorTypeLabel(static_cast<Sensor_VType>(options[x]));
    addSelector_Item(name,
                     options[x],
                     choice == static_cast<Sensor_VType>(options[x]));
  }
  addSelector_Foot();
  {
    String note;
    note = F("Changing '");
    note += outputTypeLabel;
    note += F("' may affect behavior of some controllers (e.g. Domoticz)");
    addFormNote(note);
  }
  String dummy;
  PluginCall(PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR, event, dummy);
}

void sensorTypeHelper_saveOutputSelector(struct EventStruct *event, uint8_t pconfigIndex, uint8_t valueIndex, const String& defaultValueName)
{
  if (defaultValueName.equals(ExtraTaskSettings.TaskDeviceValueNames[valueIndex])) {
    ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[valueIndex]);
  }
  pconfig_webformSave(event, pconfigIndex);
}

void pconfig_webformSave(struct EventStruct *event, uint8_t pconfigIndex)
{
  PCONFIG(pconfigIndex) = getFormItemInt(PCONFIG_LABEL(pconfigIndex), 0);
}

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const __FlashStringHelper * options[], const int indices[])
{
  uint8_t   choice = PCONFIG(pconfigIndex);
  String label  = F("Value ");

  label += (valuenr + 1);
  addFormSelector(label, PCONFIG_LABEL(pconfigIndex), optionCount, options, indices, choice);
}


void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const String options[], const int indices[])
{
  uint8_t   choice = PCONFIG(pconfigIndex);
  String label  = F("Value ");

  label += (valuenr + 1);
  addFormSelector(label, PCONFIG_LABEL(pconfigIndex), optionCount, options, indices, choice);
}
