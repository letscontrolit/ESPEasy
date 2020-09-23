#include "../../ESPEasy_common.h"
#include "../Globals/MQTT.h"

#ifdef USES_MQTT

#include "../Commands/MQTT.h"

#include "../Commands/Common.h"
#include "../Globals/Settings.h"
#include "../Globals/CPlugins.h"
#include "../Globals/ESPEasy_Scheduler.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/Scheduler.h"

#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy_Log.h"



String Command_MQTT_Publish(struct EventStruct *event, const char *Line)
{
  // ToDo TD-er: Not sure about this function, but at least it sends to an existing MQTTclient
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(enabledMqttController)) {
    return F("No MQTT controller enabled");
  }

  // Command structure:  Publish,<topic>,<value>
  String topic = parseStringKeepCase(Line, 2);
  String value = tolerantParseStringKeepCase(Line, 3);
  addLog(LOG_LEVEL_DEBUG, String(F("Publish: ")) + topic + value);

  if ((topic.length() > 0) && (value.length() > 0)) {

    bool mqtt_retainFlag;
    {
      // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
      MakeControllerSettings(ControllerSettings);
      if (!AllocatedControllerSettings()) {
        String error = F("MQTT : Cannot publish, out of RAM");
        addLog(LOG_LEVEL_ERROR, error);
        return error;
      }

      LoadControllerSettings(enabledMqttController, ControllerSettings);
      mqtt_retainFlag = ControllerSettings.mqtt_retainFlag();
    }


    // @giig1967g: if payload starts with '=' then treat it as a Formula and evaluate accordingly
    // The evaluated value is already present in event->Par2
    // FIXME TD-er: Is the evaluated value always present in event->Par2 ?
    // Should it already be evaluated, or should we evaluate it now?

    bool success = false;
    if (value[0] != '=') {
      success = MQTTpublish(enabledMqttController, topic.c_str(), value.c_str(), mqtt_retainFlag);
    }
    else {
      success = MQTTpublish(enabledMqttController, topic.c_str(), String(event->Par2).c_str(), mqtt_retainFlag);
    }
    if (success) {
      return return_command_success();
    }
  }
  return return_command_failed();
}


boolean MQTTsubscribe(controllerIndex_t controller_idx, const char* topic, boolean retained)
{
  if (MQTTclient.subscribe(topic)) {
    Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
    String log = F("Subscribed to: ");  log += topic;
    addLog(LOG_LEVEL_INFO, log);
    return true;
  }
  addLog(LOG_LEVEL_ERROR, F("MQTT : subscribe failed"));
  return false;
}

String Command_MQTT_Subscribe(struct EventStruct *event, const char* Line)
{
  if (MQTTclient.connected() ) {
    // ToDo TD-er: Not sure about this function, but at least it sends to an existing MQTTclient
    controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();
    if (validControllerIndex(enabledMqttController)) {
      bool mqtt_retainFlag;
      {
        // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
        MakeControllerSettings(ControllerSettings);
        if (!AllocatedControllerSettings()) {
          String error = F("MQTT : Cannot subscribe, out of RAM");
          addLog(LOG_LEVEL_ERROR, error);
          return error;
        }
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        mqtt_retainFlag = ControllerSettings.mqtt_retainFlag();
      }

      String eventName = Line;
      String topic = eventName.substring(10);
      if (!MQTTsubscribe(enabledMqttController, topic.c_str(), mqtt_retainFlag))
         return_command_failed();
      return_command_success();
    }
    return F("No MQTT controller enabled");
  }
  return return_not_connected();
}


#endif // ifdef USES_MQTT
