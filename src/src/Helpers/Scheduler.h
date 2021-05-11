#ifndef HELPERS_SCHEDULER_H
#define HELPERS_SCHEDULER_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/EventStructCommandWrapper.h"
#include "../DataStructs/SystemTimerStruct.h"
#include "../DataTypes/ProtocolIndex.h"
#include "../Helpers/msecTimerHandlerStruct.h"

#include <list>
#include <map>


class ESPEasy_Scheduler {
public:

  // ********************************************************************************
  //   Timers used in the scheduler
  // ********************************************************************************

  enum class IntervalTimer_e {
    TIMER_20MSEC,
    TIMER_100MSEC,
    TIMER_1SEC,
    TIMER_30SEC,
    TIMER_MQTT,
    TIMER_STATISTICS,
    TIMER_GRATUITOUS_ARP,
    TIMER_MQTT_DELAY_QUEUE,
    TIMER_C001_DELAY_QUEUE,
    TIMER_C003_DELAY_QUEUE,
    TIMER_C004_DELAY_QUEUE,
    TIMER_C007_DELAY_QUEUE,
    TIMER_C008_DELAY_QUEUE,
    TIMER_C009_DELAY_QUEUE,
    TIMER_C010_DELAY_QUEUE,
    TIMER_C011_DELAY_QUEUE,
    TIMER_C012_DELAY_QUEUE,
    TIMER_C013_DELAY_QUEUE,
    TIMER_C014_DELAY_QUEUE,
    TIMER_C015_DELAY_QUEUE,
    TIMER_C016_DELAY_QUEUE,
    TIMER_C017_DELAY_QUEUE,
    TIMER_C018_DELAY_QUEUE,
    TIMER_C019_DELAY_QUEUE,
    TIMER_C020_DELAY_QUEUE,
    TIMER_C021_DELAY_QUEUE,
    TIMER_C022_DELAY_QUEUE,
    TIMER_C023_DELAY_QUEUE,
    TIMER_C024_DELAY_QUEUE,
    TIMER_C025_DELAY_QUEUE,
    // When extending this, search for EXTEND_CONTROLLER_IDS 
    // in the code to find all places that need to be updated too.

  };

  static String toString(IntervalTimer_e timer);

  enum class PluginPtrType {
    TaskPlugin,
    ControllerPlugin,
    NotificationPlugin
  };

  enum class IntendedRebootReason_e {
    DeepSleep,
    DelayedReboot,
    ResetFactory,
    ResetFactoryPinActive,
    ResetFactoryCommand,
    CommandReboot,
    RestoreSettings,
    OTA_error,
    ConnectionFailuresThreshold,

  };

  static String toString(IntendedRebootReason_e reason);


  void markIntendedReboot(IntendedRebootReason_e reason);

  /*********************************************************************************************\
  * Generic Timer functions.
  \*********************************************************************************************/
  void setNewTimerAt(unsigned long id,
                     unsigned long timer);

  // Mix timer type int with an ID describing the scheduled job.
  static unsigned long getMixedId(unsigned long timerType,
                                  unsigned long id);

  static unsigned long decodeSchedulerId(unsigned long  mixed_id,
                                         unsigned long& timerType);

  static String        decodeSchedulerId(unsigned long mixed_id);

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

  void                 setIntervalTimer(IntervalTimer_e id);
  void                 setIntervalTimerAt(IntervalTimer_e id,
                                          unsigned long   newtimer);
  void                 setIntervalTimerOverride(IntervalTimer_e id,
                                                unsigned long   msecFromNow);

  void                 scheduleNextDelayQueue(IntervalTimer_e id,
                                              unsigned long   nextTime);

  void                 setIntervalTimer(IntervalTimer_e id,
                                        unsigned long   lasttimer);

  void                 sendGratuitousARP_now();

  void                 process_interval_timer(IntervalTimer_e id,
                                              unsigned long   lasttimer);

  /*********************************************************************************************\
  * Plugin Task Timer
  \*********************************************************************************************/
  static unsigned long createPluginTaskTimerId(deviceIndex_t deviceIndex,
                                               int           Par1);

  void                 setPluginTaskTimer(unsigned long msecFromNow,
                                          taskIndex_t   taskIndex,
                                          int           Par1,
                                          int           Par2 = 0,
                                          int           Par3 = 0,
                                          int           Par4 = 0,
                                          int           Par5 = 0);

  void process_plugin_task_timer(unsigned long id);


  /*********************************************************************************************\
  * Rules Timer
  \*********************************************************************************************/

  static unsigned long createRulesTimerId(unsigned int timerIndex);

  // Set timer for Rules#Timer events.
  // @param msecFromNow   Number of milli seconds from now (also used as interval for recurring)
  // @param timerIndex    The index of the timer used. (1 ... max)
  // @param recurringCount  Number of times needed to run (-1 for always)
  bool setRulesTimer(unsigned long msecFromNow,
                     unsigned int  timerIndex,
                     int           recurringCount = 0);

  void process_rules_timer(unsigned long id,
                           unsigned long lasttimer);

  bool pause_rules_timer(unsigned long timerIndex);

  bool resume_rules_timer(unsigned long timerIndex);


  /*********************************************************************************************\
  * Plugin Timer
  \*********************************************************************************************/
  static unsigned long createPluginTimerId(deviceIndex_t deviceIndex,
                                           int           Par1);

  void                 setPluginTimer(unsigned long msecFromNow,
                                      pluginID_t    pluginID,
                                      int           Par1,
                                      int           Par2 = 0,
                                      int           Par3 = 0,
                                      int           Par4 = 0,
                                      int           Par5 = 0);

  void process_plugin_timer(unsigned long id);


  /*********************************************************************************************\
  * GPIO Timer
  * Special timer to handle timed GPIO actions
  \*********************************************************************************************/
  static unsigned long createGPIOTimerId(byte GPIOType,
                                         byte pinNumber,
                                         int  Par1);


  void setGPIOTimer(unsigned long msecFromNow,
                    pluginID_t    pluginID,
                    int           Par1,
                    int           Par2 = 0,
                    int           Par3 = 0,
                    int           Par4 = 0,
                    int           Par5 = 0);

  void process_gpio_timer(unsigned long id);

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

  void process_task_device_timer(unsigned long task_index,
                                 unsigned long lasttimer);

  /*********************************************************************************************\
  * System Event Timer
  * Handling of these events will be asynchronous and being called from the loop().
  * Thus only use these when the result is not needed immediately.
  * Proper use case is calling from a callback function, since those cannot use yield() or delay()
  \*********************************************************************************************/

  // Note: event will be moved
  void schedule_plugin_task_event_timer(deviceIndex_t       DeviceIndex,
                                        byte                Function,
                                        struct EventStruct &&event);

  void schedule_mqtt_plugin_import_event_timer(deviceIndex_t   DeviceIndex,
                                               taskIndex_t     TaskIndex,
                                               byte            Function,
                                               char           *c_topic,
                                               byte           *b_payload,
                                               unsigned int    length);


  // Note: the event will be moved
  void schedule_controller_event_timer(protocolIndex_t     ProtocolIndex,
                                       byte                Function,
                                       struct EventStruct &&event);

  void schedule_mqtt_controller_event_timer(protocolIndex_t ProtocolIndex,
                                            CPlugin::Function Function,
                                            char           *c_topic,
                                            byte           *b_payload,
                                            unsigned int    length);

  // Note: The event will be moved
  void schedule_notification_event_timer(byte                NotificationProtocolIndex,
                                         NPlugin::Function   Function,
                                         struct EventStruct &&event);


  static unsigned long createSystemEventMixedId(PluginPtrType ptr_type,
                                                uint16_t      crc16);

  static unsigned long createSystemEventMixedId(PluginPtrType ptr_type,
                                                byte          Index,
                                                byte          Function);

  // Note, the event will be moved
  void schedule_event_timer(PluginPtrType       ptr_type,
                            byte                Index,
                            byte                Function,
                            struct EventStruct &&event);

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
