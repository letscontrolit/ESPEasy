#ifndef HELPERS_SCHEDULER_H
#define HELPERS_SCHEDULER_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/EventStructCommandWrapper.h"
#include "../DataStructs/SystemTimerStruct.h"
#include "../DataTypes/SchedulerPluginPtrType.h"
#include "../DataTypes/ProtocolIndex.h"
#include "../DataTypes/IntendedRebootReason.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../Helpers/msecTimerHandlerStruct.h"


#include "../DataStructs/SchedulerTimerID.h"
#include "../DataTypes/SchedulerIntervalTimer.h"


#include <list>
#include <map>


class ESPEasy_Scheduler {
public:


  void          markIntendedReboot(IntendedRebootReason_e reason);


  /*********************************************************************************************\
  * Generic Timer functions.
  \*********************************************************************************************/
  void          setNewTimerAt(SchedulerTimerID timerID,
                              unsigned long timer);

  static String        decodeSchedulerId(SchedulerTimerID timerID);

  /*********************************************************************************************\
  * Handle scheduled timers.
  \*********************************************************************************************/
  void                 handle_schedule();

  /*********************************************************************************************\
  * Interval Timer
  * These timers set a new scheduled timer, based on the old value.
  * This will make their interval as constant as possible.
  \*********************************************************************************************/
  void                 setNextTimeInterval(unsigned long     & timer,
                                           const unsigned long step);

  void                 setNextStrictTimeInterval(unsigned long     & timer,
                                                 const unsigned long step);

  void                 setIntervalTimer(SchedulerIntervalTimer_e id);
  void                 setIntervalTimerAt(SchedulerIntervalTimer_e id,
                                          unsigned long   newtimer);
  void                 setIntervalTimerOverride(SchedulerIntervalTimer_e id,
                                                unsigned long   msecFromNow);

  void                 scheduleNextDelayQueue(SchedulerIntervalTimer_e id,
                                              unsigned long   nextTime);

  void                 setIntervalTimer(SchedulerIntervalTimer_e id,
                                        unsigned long   lasttimer);

  void                 sendGratuitousARP_now();

  void                 process_interval_timer(SchedulerTimerID timerID,
                                              unsigned long   lasttimer);

  /*********************************************************************************************\
  * Plugin Task Timer  (PLUGIN_TASKTIMER_IN)
  * Can be scheduled per combo taskIndex & Par1 (20 least significant bits)
  \*********************************************************************************************/
  void                 setPluginTaskTimer(unsigned long msecFromNow,
                                          taskIndex_t   taskIndex,
                                          int           Par1,
                                          int           Par2 = 0,
                                          int           Par3 = 0,
                                          int           Par4 = 0,
                                          int           Par5 = 0);

  void process_plugin_task_timer(SchedulerTimerID timerID);


  /*********************************************************************************************\
  * Rules Timer
  \*********************************************************************************************/
  // Set timer for Rules#Timer events.
  // @param msecFromNow   Number of milli seconds from now (also used as interval for recurring)
  // @param timerIndex    The index of the timer used. (1 ... max)
  // @param recurringCount  Number of times needed to run (-1 for always)
  bool setRulesTimer(unsigned long msecFromNow,
                     unsigned int  timerIndex,
                     int           recurringCount = 0);

  void process_rules_timer(SchedulerTimerID timerID,
                           unsigned long lasttimer);

  bool pause_rules_timer(unsigned long timerIndex);

  bool resume_rules_timer(unsigned long timerIndex);


  /*********************************************************************************************\
  * Plugin Timer  (PLUGIN_DEVICETIMER_IN)
  * Does not reflect a specific task, but rather a plugin.
  * Can be scheduled per combo deviceIndex & Par1 (20 least significant bits)
  \*********************************************************************************************/
  void                 setPluginTimer(unsigned long msecFromNow,
                                      pluginID_t    pluginID,
                                      int           Par1,
                                      int           Par2 = 0,
                                      int           Par3 = 0,
                                      int           Par4 = 0,
                                      int           Par5 = 0);

  void process_plugin_timer(SchedulerTimerID timerID);


  /*********************************************************************************************\
  * GPIO Timer
  * Special timer to handle timed GPIO actions
  \*********************************************************************************************/
  void setGPIOTimer(unsigned long msecFromNow,
                    pluginID_t    pluginID,
                    int           pinnr,
                    int           state = 0,
                    int           repeatInterval = 0,
                    int           recurringCount = 0,
                    int           alternateInterval = 0);

  void clearGPIOTimer(pluginID_t pluginID, int pinnr);

  void process_gpio_timer(SchedulerTimerID timerID, unsigned long lasttimer);

  /*********************************************************************************************\
  * Task Device Timer
  * This is the interval set in a plugin to get a new reading.
  * These timers will re-schedule themselves as long as the plugin task is enabled.
  * When the plugin task is initialized, a call to schedule_task_device_timer_at_init
  * will bootstrap this sequence.
  \*********************************************************************************************/
  void schedule_task_device_timer_at_init(unsigned long task_index);

  // Typical use case is to run this when all needed connections are made.
  void schedule_all_task_device_timers();

  // Schedule a call to SensorSendTask, which calls PLUGIN_READ
  void schedule_task_device_timer(unsigned long task_index,
                                  unsigned long runAt);

  // Reschedule task device timer based on the set task interval.
  void reschedule_task_device_timer(unsigned long task_index,
                                    unsigned long lasttimer);

  void process_task_device_timer(SchedulerTimerID timerID,
                                 unsigned long lasttimer);

  /*********************************************************************************************\
  * System Event Timer
  * Handling of these events will be asynchronous and being called from the loop().
  * Thus only use these when the result is not needed immediately.
  * Proper use case is calling from a callback function, since those cannot use yield() or delay()
  \*********************************************************************************************/

  // Note: event will be moved
  void schedule_plugin_task_event_timer(deviceIndex_t        DeviceIndex,
                                        uint8_t              Function,
                                        struct EventStruct&& event);

#if FEATURE_MQTT
  void schedule_mqtt_plugin_import_event_timer(deviceIndex_t DeviceIndex,
                                               taskIndex_t   TaskIndex,
                                               uint8_t       Function,
                                               char         *c_topic,
                                               uint8_t      *b_payload,
                                               unsigned int  length);
#endif


  // Note: the event will be moved
  void schedule_controller_event_timer(protocolIndex_t      ProtocolIndex,
                                       uint8_t              Function,
                                       struct EventStruct&& event);

#if FEATURE_MQTT
  void schedule_mqtt_controller_event_timer(protocolIndex_t   ProtocolIndex,
                                            CPlugin::Function Function,
                                            char             *c_topic,
                                            uint8_t          *b_payload,
                                            unsigned int      length);
#endif

  // Note: The event will be moved
#if FEATURE_NOTIFIER
  void schedule_notification_event_timer(uint8_t              NotificationProtocolIndex,
                                         NPlugin::Function    Function,
                                         struct EventStruct&& event);
#endif


  // Create mixed ID for scheduling a system event to be handled by the scheduler.
  // ptr_type: Indicating whether it should be handled by controller, plugin or notifier
  // Index   : DeviceIndex / ProtocolIndex / NotificationProtocolIndex  (thus not the Plugin_ID/CPlugin_ID/NPlugin_ID, saving an extra lookup when processing)
  // Function: The function to be called for handling the event.
  // Note, the event will be moved
  void schedule_event_timer(SchedulerPluginPtrType_e        ptr_type,
                            uint8_t              Index,
                            uint8_t              Function,
                            struct EventStruct&& event);

  void process_system_event_queue();


  /*********************************************************************************************\
  * Statistics
  \*********************************************************************************************/

  String getQueueStats();

  void   updateIdleTimeStats();

  float  getIdleTimePct() const;

  void   setEcoMode(bool enabled);

private:

  // Map mixed timer ID to system timer struct.
  // N.B. Must use Mixed timer ID, similar to how it is handled in the scheduler.
  std::map<unsigned long, systemTimerStruct>systemTimers;

  msecTimerHandlerStruct msecTimerHandler;

  std::list<EventStructCommandWrapper>ScheduledEventQueue;

  unsigned long last_system_event_run         = 0;
  unsigned long timer_gratuitous_arp_interval = 5000;
};

#endif // HELPERS_SCHEDULER_H
