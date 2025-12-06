#include "../Helpers/Scheduler.h"

#include "../DataStructs/Scheduler_SystemEventQueueTimerID.h"
#include "../DataStructs/TimingStats.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Device.h"
#include "../Globals/NPlugins.h"
#include "../Globals/RTC.h"

#include "../Helpers/_Plugin_init.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

/*********************************************************************************************\
* System Event Timer
* Handling of these events will be asynchronous and being called from the loop().
* Thus only use these when the result is not needed immediately.
* Proper use case is calling from a callback function, since those cannot use yield() or delay()
\*********************************************************************************************/
void ESPEasy_Scheduler::schedule_plugin_task_event_timer(
  deviceIndex_t        DeviceIndex,
  uint8_t              Function,
  struct EventStruct&& event) {
  if (validDeviceIndex(DeviceIndex)) {
    schedule_event_timer(
      SchedulerPluginPtrType_e::TaskPlugin,
      DeviceIndex.value,
      Function,
      std::move(event));
  }
}

#if FEATURE_MQTT
void ESPEasy_Scheduler::schedule_mqtt_plugin_import_event_timer(
  deviceIndex_t  DeviceIndex,
  taskIndex_t    TaskIndex,
  uint8_t        Function,
  const char    *c_topic,
  const uint8_t *b_payload,
  unsigned int   length) {
  if (validDeviceIndex(DeviceIndex)) {
    EventStruct  event(TaskIndex);
    const size_t topic_length = strlen_P(c_topic);

    if (!(reserve_special(event.String1, topic_length) &&
          reserve_special(event.String2, length))) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
      return;
    }
    event.String1.concat(c_topic, topic_length);
    event.String2.concat((const char *)b_payload, length);

    // Emplace using move.
    // This makes sure the relatively large event will not be in memory twice.
    const SystemEventQueueTimerID timerID(
      SchedulerPluginPtrType_e::TaskPlugin,
      DeviceIndex.value,
      static_cast<uint8_t>(Function));

    {
      // Make sure the emplace_back is not using 2nd heap
      # ifdef USE_SECOND_HEAP
      HeapSelectDram ephemeral;
      # endif // ifdef USE_SECOND_HEAP

      ScheduledEventQueue.emplace_back(timerID.mixed_id, std::move(event));
    }
  }
}

#endif // if FEATURE_MQTT

void ESPEasy_Scheduler::schedule_controller_event_timer(
  protocolIndex_t      ProtocolIndex,
  uint8_t              Function,
  struct EventStruct&& event) {
  if (validProtocolIndex(ProtocolIndex)) {
    schedule_event_timer(
      SchedulerPluginPtrType_e::ControllerPlugin,
      ProtocolIndex,
      Function,
      std::move(event));
  }
}

#if FEATURE_MQTT
void ESPEasy_Scheduler::schedule_mqtt_controller_event_timer(
  protocolIndex_t   ProtocolIndex,
  CPlugin::Function Function,
  const char       *c_topic,
  const uint8_t    *b_payload,
  unsigned int      length) {
  if (validProtocolIndex(ProtocolIndex) && c_topic && b_payload) {
    EventStruct  event;
    const size_t topic_length = strlen_P(c_topic);

    // This is being called from a callback function, so do not try to allocate this on the 2nd heap, but rather on the default heap.
    if (!(reserve_special(event.String1, topic_length) &&
          reserve_special(event.String2, length))) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
      return;
    }
    event.String1.concat(c_topic, topic_length);
    event.String2.concat((const char *)b_payload, length);

    // Emplace using move.
    // This makes sure the relatively large event will not be in memory twice.
    const SystemEventQueueTimerID timerID(
      SchedulerPluginPtrType_e::ControllerPlugin,
      ProtocolIndex,
      static_cast<uint8_t>(Function));

    {
      # ifdef USE_SECOND_HEAP
      // Make sure emplace_back is not called when on 2nd heap
      HeapSelectDram ephemeral;
      # endif // ifdef USE_SECOND_HEAP
      
      ScheduledEventQueue.emplace_back(timerID.mixed_id, std::move(event));
    }
  }
}

#endif // if FEATURE_MQTT

#if FEATURE_NOTIFIER
void ESPEasy_Scheduler::schedule_notification_event_timer(
  uint8_t              NotificationProtocolIndex,
  NPlugin::Function    Function,
  struct EventStruct&& event)
{
  schedule_event_timer(SchedulerPluginPtrType_e::NotificationPlugin,
                       NotificationProtocolIndex,
                       static_cast<uint8_t>(Function),
                       std::move(event));
}

#endif // if FEATURE_NOTIFIER

void ESPEasy_Scheduler::schedule_event_timer(
  SchedulerPluginPtrType_e ptr_type,
  uint8_t                  Index,
  uint8_t                  Function,
  struct EventStruct    && event) {
  const SystemEventQueueTimerID timerID(ptr_type, Index, Function);

  //  EventStructCommandWrapper eventWrapper(mixedId, *event);
  //  ScheduledEventQueue.push_back(eventWrapper);

  // Make sure emplace_back is not done on the 2nd heap
  # ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  # endif // ifdef USE_SECOND_HEAP
  ScheduledEventQueue.emplace_back(timerID.mixed_id, std::move(event));
}

void ESPEasy_Scheduler::process_system_event_queue() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  if (ScheduledEventQueue.size() == 0) { return; }

  START_TIMER

  const SchedulerTimerID timerID(ScheduledEventQueue.front().id);
  const SystemEventQueueTimerID *sysEventTimerID =
    reinterpret_cast<const SystemEventQueueTimerID *>(&timerID);


  if (RTC.lastMixedSchedulerId != timerID.mixed_id) {
    RTC.lastMixedSchedulerId = timerID.mixed_id;
    saveToRTC();
  }

  uint8_t Function                  = sysEventTimerID->getFunction();
  uint8_t Index                     = sysEventTimerID->getIndex();
  SchedulerPluginPtrType_e ptr_type = sysEventTimerID->getPtrType();

  // At this moment, the String is not being used in the plugin calls, so just supply a dummy String.
  // Also since these events will be processed asynchronous, the resulting
  //   output in the String is probably of no use elsewhere.
  // Else the line string could be used.
  String tmpString;

  switch (ptr_type) {
    case SchedulerPluginPtrType_e::TaskPlugin:
    {
      const deviceIndex_t deviceIndex = deviceIndex_t::toDeviceIndex(Index);

      if (validDeviceIndex(deviceIndex)) {
        if (((Function != PLUGIN_READ) &&
             (Function != PLUGIN_MQTT_CONNECTION_STATE) &&
             (Function != PLUGIN_MQTT_IMPORT))
            || Device[deviceIndex].ErrorStateValues) {
          // FIXME TD-er: LoadTaskSettings should only be called when needed, not pre-emptive.
          LoadTaskSettings(ScheduledEventQueue.front().event.TaskIndex);
        }
        PluginCall(deviceIndex,
                   Function,
                   &ScheduledEventQueue.front().event,
                   tmpString);
      }
      break;
    }
    case SchedulerPluginPtrType_e::ControllerPlugin:
      CPluginCall(Index,
                  static_cast<CPlugin::Function>(Function),
                  &ScheduledEventQueue.front().event,
                  tmpString);
      break;
#if FEATURE_NOTIFIER
    case SchedulerPluginPtrType_e::NotificationPlugin:
      NPlugin_ptr[Index](static_cast<NPlugin::Function>(Function),
                         &ScheduledEventQueue.front().event,
                         tmpString);
      break;
#endif // if FEATURE_NOTIFIER
  }
  ScheduledEventQueue.pop_front();
  STOP_TIMER(PROCESS_SYSTEM_EVENT_QUEUE);
}
