#include "../Helpers/TaskValuesWriterHelper.h"

#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"

#if FEATURE_STRING_VARIABLES
# include "../Globals/Device.h"
#endif // #if FEATURE_STRING_VARIABLES
#include "../Globals/Plugins.h"
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
# include "../Helpers/ESPEasy_UnitOfMeasure.h"
#endif
#include "../WebServer/JSON.h"

TaskValuesWriterHelper::TaskValuesWriterHelper(struct EventStruct *e)
  : event(e) {
  if (e) {
    const pluginID_t pid = Settings.getPluginID_for_task(event->TaskIndex);

    if (validPluginID_fullcheck(pid)) {
      deviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);
      valueCount  = getValueCountForTask(event->TaskIndex);
    }
  }
}

void TaskValuesWriterHelper::clear()
{
  valName.clear();
  valName_id.clear();
  value.clear();
  value_id.clear();
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  uom.clear();
#endif
#if FEATURE_STRING_VARIABLES
  hasPresentation = false;
  presentation.clear();
#endif // if FEATURE_STRING_VARIABLES
  nrDecimals = 0;
  addTrailingBreak = false;
}

void TaskValuesWriterHelper::setID(uint8_t varNr)
{
  if (!validDeviceIndex(deviceIndex)) { return; }
  valueNumber = varNr;
  const String id_postfix = strformat(F("%u_%u"), event->TaskIndex, varNr);
  valName_id = concat(F("valuename_"), id_postfix);
  value_id   = concat(F("value_"), id_postfix);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE

  if (!uom.isEmpty()) {
    uom_id = concat(F("uom_"), id_postfix);
  }
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
#if FEATURE_STRING_VARIABLES

  if (hasPresentation) {
    presentation_id = concat(F("pres_"), id_postfix);
  }
#endif // if FEATURE_STRING_VARIABLES
}

void TaskValuesWriterHelper::write()
{
  if (!validDeviceIndex(deviceIndex)) { return; }

  if (event->kvWriter) { handle_json_stream_task_value_data(this); }
  else { pluginWebformShowValue(); }
}

void TaskValuesWriterHelper::writeTaskValues()
{
  if (!validDeviceIndex(deviceIndex)) { return; }
  String customValuesString;

  if (PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, event, customValuesString)) {
    return;
  }

  writeRegularTaskValues();
#if FEATURE_STRING_VARIABLES
  writeDerivedTaskValues();
#endif
}

void TaskValuesWriterHelper::writeCustom(uint8_t varNr, const __FlashStringHelper * label, const String& val, bool addTrailing_Break)
{
    writeCustom(varNr, String(label), val, addTrailing_Break);
}

void TaskValuesWriterHelper::writeCustom(uint8_t varNr, const String& label, const String& val, bool addTrailing_Break)
{
  if (!validDeviceIndex(deviceIndex)) { return; }
  clear();
  valName = label;
  value = val;
  addTrailingBreak = addTrailing_Break;
  setID(varNr);
  write();
}

void TaskValuesWriterHelper::writeRegularTaskValues()
{
  if (!validDeviceIndex(deviceIndex)) { return; }

  for (uint8_t x = 0; x < valueCount; x++)
  {
    if (initRegularTaskValue(x)) {
      write();
    }
  }
}

#if FEATURE_STRING_VARIABLES

void TaskValuesWriterHelper::writeDerivedTaskValues()
{
  if (!validDeviceIndex(deviceIndex)) { return; }
  const DeviceStruct& device = Device[deviceIndex];

  int varNr = VARS_PER_TASK;

  if (Settings.ShowDerivedTaskValues(event->TaskIndex)) {
    String taskName = getTaskDeviceName(event->TaskIndex);
    taskName.toLowerCase();
    String postfix;
    const String search = getDerivedValueSearchAndPostfix(taskName, postfix);

    auto it = customStringVar.begin();

    while (it != customStringVar.end()) {
      if (it->first.startsWith(search) && it->first.endsWith(postfix)) {
        clear();
        valName = it->first.substring(search.length(), it->first.indexOf('-'));
        String vType;
        const String vname2 = getDerivedValueNameUomAndVType(taskName, valName, uom, vType);

        if (!vname2.isEmpty()) {
          valName = vname2;
        }

        if (!it->second.isEmpty()) {
          value = it->second;

          // FIXME TD-er: Why these differences between JSON and Web?
          if (event->kvWriter)
          {
            stripEscapeCharacters(value);
            value        = parseTemplate(value);
            nrDecimals   = 255; // FIXME Use the minimal number of decimals needed
            presentation =
              formatUserVarForPresentation(
                event,
                INVALID_TASKVAR_INDEX,
                hasPresentation,
                value,
                deviceIndex,
                valName);
          } else {
            value        = parseTemplateAndCalculate(value);
            presentation = getCustomStringVar(strformat(
                                                F(TASK_VALUE_PRESENTATION_PREFIX_TEMPLATE),
                                                taskName.c_str(),
                                                valName.c_str()));

            if (!presentation.isEmpty()) {
              stripEscapeCharacters(presentation);
              presentation.replace(F("%value%"), value);
              presentation = parseTemplate(presentation);
            }
          }

          setID(varNr);
          write();
          ++varNr;
        }
      }

      // FIXME TD-er: Search for "compareTo(search)" in the code to find lots of nearly duplicate code
      else if (it->first.substring(0, search.length()).compareTo(search) > 0) {
        break;
      }
      ++it;
    }
  }
}

#endif // if FEATURE_STRING_VARIABLES

void TaskValuesWriterHelper::pluginWebformShowValue()
{
if (!validDeviceIndex(deviceIndex)) return;
  if (valueNumber > 0) {
    addHtmlDiv(F("div_br"));
  }
  if (hasPresentation) value = presentation;
  else if (!uom.isEmpty())
   value += concat(' ', uom);

  String valName_tmp(valName);

  if (!valName_tmp.endsWith(F(":"))) {
    valName_tmp += ':';
  }
  addHtmlDiv(F("div_l"), valName_tmp, valName_id);
  addHtmlDiv(F("div_r"), value,       value_id);

  if (addTrailingBreak) {
    addHtmlDiv(F("div_br"));
  }
}


bool TaskValuesWriterHelper::initRegularTaskValue(uint8_t varNr)
{
  if (!validDeviceIndex(deviceIndex)) { return false; }

  if (varNr >= valueCount) { return false; }

  valueNumber = varNr;

  // Make sure we can re-use the same struct for multiple task values
  clear();

  // Need to produce the 'regular' task values
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE

  if (Settings.ShowUnitOfMeasureOnDevicesPage()) {
    const uint8_t uomIndex = Cache.getTaskVarUnitOfMeasure(event->TaskIndex, varNr);

    if ((uomIndex != 0)) {
      uom = toUnitOfMeasureName(uomIndex);
    }
  }
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

  nrDecimals = Cache.getTaskDeviceValueDecimals(event->TaskIndex, varNr);

  value = formatUserVarNoCheck(event, varNr);

  #if FEATURE_STRING_VARIABLES
  const DeviceStruct& device = Device[deviceIndex];
  bool hasPresentation       = false;

  if (!device.HideDerivedValues) {
    presentation = formatUserVarForPresentation(event, varNr, hasPresentation, value, deviceIndex);
  }
  #endif // if FEATURE_STRING_VARIABLES
  valName = Cache.getTaskDeviceValueName(event->TaskIndex, varNr);
  setID(varNr);

  return true;
}

