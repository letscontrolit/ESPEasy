#ifndef HELPERS_SCHEDULER_H
#define HELPERS_SCHEDULER_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/EventStructCommandWrapper.h"
#include "../DataStructs/SchedulerTimerID.h"
#include "../DataStructs/SystemTimerStruct.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/IntendedRebootReason.h"
#include "../DataTypes/ProtocolIndex.h"
#include "../DataTypes/SchedulerIntervalTimer.h"
#include "../DataTypes/SchedulerPluginPtrType.h"

#include "../Helpers/msecTimerHandlerStruct.h"

#include <list>
#include <map>



  /*********************************************************************************************\
  * ESPEasy uses a scheduler, which is essentially its heart beat.
  * 
  * It is basically a list of tuples with:
  * - timestamp (in msec)
  * - 32-bit value describing what should be done.
  * 
  * This list is sorted on timestamp, with the next scheduled action at the front.
  * 
  *   Scheduled Action Parameters
  *   ---------------------------
  * 
  * The 32-bit value uses a few bits to signify its timer type.
  * Per timer type the left over bits can be used to store some arguments.
  *  
  * Some timer types need to store more which cannot be stored in this 32-bit value.
  * For example system timers (e.g. a timer started from rules) need more parameters.
  * These will be stored in a separate map, where this 32-bit value is used as key to access these arguments.
  * As it is stored in a map, this 32-bit value for this timer type needs to be unique.
  * To make those values unique, some of the arguments are also stored in this 32-bit value.
  * For example "Par1" may be used to make this more unique. 
  * For GPIO longpulse the rising and falling edge can already be scheduled by including the pin state in this 32-bit value.
  * 
  *   Background Actions & System/Rules Events
  *   ----------------------------------------
  * 
  * Whenever timestamp of the first item in this actions list is not yet due, 
  * the scheduler may perform background tasks or call delay() to reduce power consumption.
  * 
  * N.B. These background tasks will also be executed at some minimal guaranteed interval, 
  *      to make sure a fully loaded ESP will not stall as background work piles up.
  * 
  * Some actions do not have a specific scheduled timer, as they just have to be performed as soon as possible.
  * For example processing rules events are put in a separate event queue.
  * The Scheduler tries to find a good balance between processing such queued items 
  * and making sure scheduled actions will be done as close as possible to their scheduled moment.
  * 
  *   Fixed Interval 'jitter'
  *   -----------------------
  * 
  * A lot of ESPEasy's operations consists of repetitive actions.
  * These often have a specific interval, like calls to PLUGIN_TEN_PER_SECOND.
  * Also each task has its own configured interval.
  *  
  * Whenever an interval based scheduled action is running behind its schedule, 
  * the scheduler will try to keep up with its original pace.
  * For example:
  * A call to PLUGIN_TEN_PER_SECOND is scheduled to run at time X.
  * Whenever it is being processed, the first thing to do is to schedule it at time X + 100 msec.
  * If the ESP is running behind, this new timestamp could already be in the past.
  * The scheduler will then try to get in sync again, unless the scheduler missed more then 1 full interval.
  * If this happens, the scheduler will just 'restart' the interval considering the current timestamp as start of the interval.
  * 
  * This will eventually spread scheduled intervals to their optimum interval cadance.
  * However it may appear some scheduled actions may drift apart where they may have been running nearly in sync before.
  * 
  * If actions should be executed in sync, one should trigger such actions from the rules.
  * For example grouping "taskRun" calls triggered via the same rules event.
  * 
  * 
  * 
  \*********************************************************************************************/
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

void                 setPluginTaskTimer(unsigned long msecFromNow,
                                          taskIndex_t   taskIndex,
                                          PluginFunctions_e function,
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
  // @param startImmediately  Run immediately
  bool setRulesTimer(unsigned long msecFromNow,
                     unsigned int  timerIndex,
                     int           recurringCount = 0,
                     bool          startImmediately = false);

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
  void schedule_mqtt_plugin_import_event_timer(deviceIndex_t  DeviceIndex,
                                               taskIndex_t    TaskIndex,
                                               uint8_t        Function,
                                               const char    *c_topic,
                                               const uint8_t *b_payload,
                                               unsigned int   length);
#endif


  // Note: the event will be moved
  void schedule_controller_event_timer(protocolIndex_t      ProtocolIndex,
                                       uint8_t              Function,
                                       struct EventStruct&& event);

#if FEATURE_MQTT
  void schedule_mqtt_controller_event_timer(protocolIndex_t   ProtocolIndex,
                                            CPlugin::Function Function,
                                            const char       *c_topic,
                                            const uint8_t    *b_payload,
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
