#include "../Controller_config/C019_config.h"

#ifdef USES_C019

# include "../Globals/Settings.h"

# define C019_FORWARDFILTERING_ELEMENTS  3

void C019_ForwardFiltering::fromStringArray(String strings[], uint8_t filterNr)
{
  const uint8_t index = C019_FORWARDFILTERING_ELEMENTS * filterNr;

  taskIndex  = strings[index + 0].toInt();
  matchTopic = std::move(strings[index + 1]);
}

void C019_ForwardFiltering::toStringArray(String strings[], uint8_t filterNr) const
{
  const uint8_t index = C019_FORWARDFILTERING_ELEMENTS * filterNr;

  strings[index + 0] = taskIndex;
  strings[index + 1] = std::move(matchTopic);
}

void C019_ConfigStruct::validate() {
  if (nrTaskFilters > TASKS_MAX) { nrTaskFilters = 0; }

  if (!validControllerIndex(forwardControllerIdx)) {
    forwardControllerIdx = INVALID_CONTROLLER_INDEX;
  }
}

void C019_ConfigStruct::reset() {
  nrTaskFilters = 0;
  filters.clear();
  forwardControllerIdx = INVALID_CONTROLLER_INDEX;
}

void C019_ConfigStruct::init(struct EventStruct *event) {
  constexpr size_t filtersOffset = offsetof(C019_ConfigStruct, filters);

  LoadCustomControllerSettings(event->ControllerIndex, reinterpret_cast<uint8_t *>(this), filtersOffset);
  filters.resize(nrTaskFilters);
  {
    const int nrStrings = nrTaskFilters * C019_FORWARDFILTERING_ELEMENTS;
    String    strings[nrStrings];
    LoadStringArray(SettingsType::Enum::CustomControllerSettings_Type,
                    event->ControllerIndex,
                    strings,
                    nrStrings,
                    0,
                    filtersOffset);

    for (uint8_t i = 0; i < nrTaskFilters; ++i) {
      filters[i].fromStringArray(strings, i);
    }
  }

  if (wifiChannel < 0) {
    wifiChannel = Settings.ForceESPEasyNOWchannel;
  }
}

void C019_ConfigStruct::webform_load(struct EventStruct *event) {
  init(event);
  addFormNumericBox(LabelType::ESPEASY_NOW_FORCED_CHANNEL, Settings.ForceESPEasyNOWchannel, 0, 14);
  addFormNote(F("Force channel to use for "
                ESPEASY_NOW_NAME
                "-only mode (0 = use any channel)"));

  addFormCheckBox(F("Forward MQTT from " ESPEASY_NOW_NAME), F("fwd_mqtt"), forwardMQTT);

  addTableSeparator(F("MQTT Forward Filtering"), 2, 3);
  addFormCheckBox(F("Filter Forward MQTT"), F("filter_fwd"), filterMQTT_forward);
  addFormNumericBox(F("Nr Filter Tasks"), F("nrfiltertasks"), nrTaskFilters, 0, TASKS_MAX);

  for (uint8_t i = 0; i < nrTaskFilters; ++i) {
    addTableSeparator(concat(F("Filter "), i + 1), 2, 2);
    addFormTextBox(F("Match Topic"),
                   concat(F("topic_match"), i),
                   filters[i].matchTopic,
                   C019_MQTT_TOPIC_LENGTH - 1);
    addRowLabel(F("Task"));
    addTaskSelect(concat(F("ftask"), i), filters[i].taskIndex, false);
  }
  addFormSeparator(2);
}

void C019_ConfigStruct::webform_save(struct EventStruct *event) {
  reset();

  forwardMQTT        = isFormItemChecked(F("fwd_mqtt"));
  filterMQTT_forward = isFormItemChecked(F("filter_fwd"));
  nrTaskFilters      = getFormItemInt(F("nrfiltertasks"));

  // FIXME TD-er: For now have it as duplicate setting...
  Settings.ForceESPEasyNOWchannel = getFormItemInt(getInternalLabel(LabelType::ESPEASY_NOW_FORCED_CHANNEL));
  wifiChannel                     = Settings.ForceESPEasyNOWchannel;


  constexpr size_t filtersOffset = offsetof(C019_ConfigStruct, filters);

  SaveCustomControllerSettings(event->ControllerIndex, reinterpret_cast<const uint8_t *>(this), filtersOffset);

  if (nrTaskFilters > 0) {
    const int nrStrings = nrTaskFilters * C019_FORWARDFILTERING_ELEMENTS;
    String    strings[nrStrings];

    for (uint8_t i = 0; i < nrTaskFilters; ++i) {
      C019_ForwardFiltering filter;

      filter.taskIndex  = getFormItemInt(concat(F("ftask"), i), INVALID_TASK_INDEX);
      filter.matchTopic = webArg(concat(F("topic_match"), i));
      filter.toStringArray(strings, i);
    }
    SaveStringArray(SettingsType::Enum::CustomControllerSettings_Type,
                    event->ControllerIndex,
                    strings,
                    nrStrings,
                    0,
                    filtersOffset);
  }
}

taskIndex_t C019_ConfigStruct::matchTopic(const String& topic) const
{
  for (auto it = filters.begin(); it != filters.end(); ++it) {
    // FIXME TD-er: Must match MQTT topic wildcards
    if (topic.indexOf(it->matchTopic) != -1) { return it->taskIndex; }
  }
  return INVALID_TASK_INDEX;
}

#endif // ifdef USES_C019
