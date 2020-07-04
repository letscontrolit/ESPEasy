#ifndef HELPERS_SCHEDULER_H
#define HELPERS_SCHEDULER_H


#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"


/*********************************************************************************************\
* Generic Timer functions.
\*********************************************************************************************/
void setTimer(unsigned long timerType, unsigned long id, unsigned long msecFromNow);

void setNewTimerAt(unsigned long id, unsigned long timer);

// Mix timer type int with an ID describing the scheduled job.
unsigned long getMixedId(unsigned long timerType, unsigned long id);

unsigned long decodeSchedulerId(unsigned long mixed_id, unsigned long& timerType);

String decodeSchedulerId(unsigned long mixed_id);

/*********************************************************************************************\
* Handle scheduled timers.
\*********************************************************************************************/
void handle_schedule();

/*********************************************************************************************\
* Interval Timer
* These timers set a new scheduled timer, based on the old value.
* This will make their interval as constant as possible.
\*********************************************************************************************/
void setNextTimeInterval(unsigned long& timer, const unsigned long step);

void setIntervalTimer(unsigned long id);
void setIntervalTimerAt(unsigned long id, unsigned long newtimer);
void setIntervalTimerOverride(unsigned long id, unsigned long msecFromNow);

void scheduleNextDelayQueue(unsigned long id, unsigned long nextTime);

void setIntervalTimer(unsigned long id, unsigned long lasttimer);

void sendGratuitousARP_now();

void process_interval_timer(unsigned long id, unsigned long lasttimer);

/*********************************************************************************************\
* Plugin Task Timer
\*********************************************************************************************/
unsigned long createPluginTaskTimerId(deviceIndex_t deviceIndex, int Par1);

/* // Not (yet) used
   void splitPluginTaskTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
   const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
   plugin = mixed_id & 0xFF;
   Par1 = (mixed_id & mask) >> 8;
   }
 */
void setPluginTaskTimer(unsigned long msecFromNow, taskIndex_t taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5);

void process_plugin_task_timer(unsigned long id);

/*********************************************************************************************\
* Plugin Timer
\*********************************************************************************************/
unsigned long createPluginTimerId(deviceIndex_t deviceIndex, int Par1);

/* // Not (yet) used
   void splitPluginTaskTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
   const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
   plugin = mixed_id & 0xFF;
   Par1 = (mixed_id & mask) >> 8;
   }
 */
void setPluginTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5);

void process_plugin_timer(unsigned long id);

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
unsigned long createGPIOTimerId(byte pinNumber, int Par1);

void setGPIOTimer(unsigned long msecFromNow, int Par1, int Par2, int Par3, int Par4, int Par5);

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

void schedule_task_device_timer(unsigned long task_index, unsigned long runAt);

void process_task_device_timer(unsigned long task_index, unsigned long lasttimer);

/*********************************************************************************************\
* System Event Timer
* Handling of these events will be asynchronous and being called from the loop().
* Thus only use these when the result is not needed immediately.
* Proper use case is calling from a callback function, since those cannot use yield() or delay()
\*********************************************************************************************/
void schedule_plugin_task_event_timer(deviceIndex_t DeviceIndex, byte Function, struct EventStruct *event);

void schedule_controller_event_timer(protocolIndex_t ProtocolIndex, byte Function, struct EventStruct *event);

void schedule_mqtt_controller_event_timer(protocolIndex_t ProtocolIndex, byte Function, char *c_topic, byte *b_payload, unsigned int length);

void schedule_notification_event_timer(byte NotificationProtocolIndex, byte Function, struct EventStruct *event);

void schedule_event_timer(PluginPtrType ptr_type, byte Index, byte Function, struct EventStruct *event);

unsigned long createSystemEventMixedId(PluginPtrType ptr_type, uint16_t crc16);

unsigned long createSystemEventMixedId(PluginPtrType ptr_type, byte Index, byte Function);

void process_system_event_queue();



#endif // HELPERS_SCHEDULER_H