#include "../Controller_config/C019_config.h"

#ifdef USES_C019


void C019_ConfigStruct::validate() {
  if (!validTaskIndex(filterTaskIndex)) {
    filterTaskIndex = INVALID_TASK_INDEX;
  }

  if (!validControllerIndex(forwardControllerIdx)) {
    forwardControllerIdx = INVALID_CONTROLLER_INDEX;
  }
}

void C019_ConfigStruct::reset() {
  filterTaskIndex = INVALID_TASK_INDEX;
  forwardControllerIdx = INVALID_CONTROLLER_INDEX;
}

void C019_ConfigStruct::webform_load() {
  addFormCheckBox(F("Forward MQTT from " ESPEASY_NOW_NAME), F("fwd_mqtt"),   forwardMQTT);

  addTableSeparator(F("MQTT Forward Filtering"), 2, 3);
  addFormCheckBox(F("Filter Forward MQTT"),                 F("filter_fwd"), filterMQTT_forward);
  addRowLabel(F("Filter Task"));
  addTaskSelect(F("ftask"), filterTaskIndex);

  addFormTextBox(F("Filter Publish Prefix"), F("pub_pref"), filterPublishPrefix, C019_MQTT_TOPIC_LENGTH - 1);
  addFormTextBox(F("Filter Subscribe"),      F("sub"),      filterSubscribe,     C019_MQTT_TOPIC_LENGTH - 1);
}

void C019_ConfigStruct::webform_save() {
  reset();

  forwardMQTT        = isFormItemChecked(F("fwd_mqtt"));
  filterMQTT_forward = isFormItemChecked(F("filter_fwd"));
  filterTaskIndex    = getFormItemInt(F("ftask"));

  strlcpy(filterPublishPrefix, webArg(F("pub_pref")).c_str(), sizeof(filterPublishPrefix));
  strlcpy(filterSubscribe,     webArg(F("sub")).c_str(),      sizeof(filterSubscribe));
}

#endif // ifdef USES_C019
