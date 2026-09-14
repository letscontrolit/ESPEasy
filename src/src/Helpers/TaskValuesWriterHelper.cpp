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
  value.clear();
  attribute.clear();
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  uom.clear();
#endif
#if FEATURE_STRING_VARIABLES
  hasPresentation = false;
  presentation.clear();
#endif // if FEATURE_STRING_VARIABLES
  nrDecimals                   = 0;
  _writeRegularTaskValuesFirst = false;
  _isLast                      = false;
}

bool TaskValuesWriterHelper::isEmpty() const
{
  return valName.isEmpty() && value.isEmpty();
}

String TaskValuesWriterHelper::format_ID(ID_type id) const
{
  if (!validDeviceIndex(deviceIndex)) { return EMPTY_STRING; }
  const __FlashStringHelper *id_str = F("");

  switch (id)
  {
    case ID_type::ValueName: id_str = F("valuename_");
      break;
    case ID_type::Value: id_str = F("value_");
      break;
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    case ID_type::UoM: id_str = F("uom_");
      break;
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
#if FEATURE_STRING_VARIABLES
    case ID_type::Presentation: id_str = F("pres_");
      break;
#endif // if FEATURE_STRING_VARIABLES
    default:
      return EMPTY_STRING;
  }
  const String id_postfix = strformat(F("%u_%u"), event->TaskIndex, valueNumber);
  return concat(id_str, id_postfix);
}

void TaskValuesWriterHelper::write()
{
  if (!validDeviceIndex(deviceIndex)) { return; }
#ifdef WEBSERVER_JSON

  if (event->kvWriter) {
    handle_json_stream_task_value_data(this);
  } else {
    pluginWebformShowValue();
  }
#else // ifdef WEBSERVER_JSON
  pluginWebformShowValue();
#endif // ifdef WEBSERVER_JSON
  clear();
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

void TaskValuesWriterHelper::writeCustom(uint8_t varNr, const __FlashStringHelper *label, const String& val, bool isLast)
{
  writeCustom(varNr, String(label), val, EMPTY_STRING, isLast);
}

void TaskValuesWriterHelper::writeCustom(uint8_t varNr, const String& label, const String& val, bool isLast)
{
  writeCustom(varNr, label, val, EMPTY_STRING, isLast);
}

void TaskValuesWriterHelper::writeCustom(
  uint8_t       varNr,
  const String& label,
  const String& val,
  ValueStruct&& attr,
  bool          isLast)
{
  if (!validDeviceIndex(deviceIndex)) { return; }

  if (_writeRegularTaskValuesFirst) {
    writeRegularTaskValues();
#if FEATURE_STRING_VARIABLES
    writeDerivedTaskValues();
#endif
  }
  valueNumber = varNr;
  _isLast     = isLast;
  valName     = label;
  value       = val;
  attribute   = std::move(attr);
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
        //        clear();
        valName = it->first.substring(search.length(), it->first.indexOf('-'));
        String vType;
        const String vname2 = getDerivedValueNameUomAndVType(taskName, valName, uom, vType);

        if (!vname2.isEmpty()) {
          valName = vname2;
        }

        if (!it->second.isEmpty()) {
          valueNumber = varNr;
          value       = it->second;

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
  if (!validDeviceIndex(deviceIndex)) { return; }

  if ((valueNumber == 0) && _preformatted) {
    addHtml(F("<pre>")); // To keep spaces etc. in the shown output
  }

  if (valueNumber > 0) {
    addHtmlDiv(F("div_br"));
  }
  String value_tmp(_preformatted ? value : stripQuotes(value));

#if FEATURE_STRING_VARIABLES

  if (hasPresentation) { value_tmp = presentation; }
  else if (!uom.isEmpty()) {
    value_tmp += concat(' ', uom);
  }
#endif // if FEATURE_STRING_VARIABLES

  addHtmlDiv(F("div_l"), valName, format_ID(ID_type::ValueName), attribute.toString());

  if (!value_tmp.isEmpty() || !_preformatted) {
    addHtmlDiv(F("div_r"), value_tmp, format_ID(ID_type::Value));
  }

  if (_isLast && _preformatted) {
    addHtml(F("</pre>")); // To keep spaces etc. in the shown output
    _preformatted = false;
    _isLast       = false;
  }
}

bool TaskValuesWriterHelper::initRegularTaskValue(uint8_t varNr)
{
  if (!validDeviceIndex(deviceIndex)) { return false; }

  if (varNr >= valueCount) { return false; }

  valueNumber = varNr;

  // Make sure we can re-use the same struct for multiple task values
  clear(); // TODO TD-er: Still needed, now we call clear() at the end of write() ?

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

  return true;
}
