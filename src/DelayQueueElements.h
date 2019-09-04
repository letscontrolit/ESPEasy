#ifndef DELAY_QUEUE_ELEMENTS_H
#define DELAY_QUEUE_ELEMENTS_H

#include "DataStructs/ControllerSettingsStruct.h"
#include "ESPEasy_fdwdecl.h"

// These element classes should be defined as class, to be used as template.

/*********************************************************************************************\
* MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element {
public:

  MQTT_queue_element() : controller_idx(0), _retained(false) {}

  MQTT_queue_element(int ctrl_idx,
                     const String& topic, const String& payload, boolean retained) :
    controller_idx(ctrl_idx), _topic(topic), _payload(payload), _retained(retained)
  {}

  size_t getSize() const {
    return sizeof(this) + _topic.length() + _payload.length();
  }

  int controller_idx;
  String _topic;
  String _payload;
  boolean _retained;
};

/*********************************************************************************************\
* Simple queue element, only storing controller index and some String
\*********************************************************************************************/
class simple_queue_element_string_only {
public:

  simple_queue_element_string_only() : controller_idx(0) {}

  simple_queue_element_string_only(int ctrl_idx, const String& req) :
    controller_idx(ctrl_idx), txt(req) {}

  size_t getSize() const {
    return sizeof(this) + txt.length();
  }

  int controller_idx;
  String txt;
};


//#ifdef USES_C001
/*********************************************************************************************\
* C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
#define C001_queue_element simple_queue_element_string_only
//#endif //USES_C001

//#ifdef USES_C003
/*********************************************************************************************\
* C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
#define C003_queue_element simple_queue_element_string_only
//#endif //USES_C003

//#ifdef USES_C004
/*********************************************************************************************\
* C004_queue_element for queueing requests for C004 ThingSpeak.
*   Typical use case for Thingspeak is to only send values every N seconds/minutes.
*   So we just store everything needed to recreate the event when the time is ready.
\*********************************************************************************************/
class C004_queue_element {
public:

  C004_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}

  C004_queue_element(const struct EventStruct *event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {
    if (sensorType == SENSOR_TYPE_STRING) {
      txt = event->String2;
    }
  }

  size_t getSize() const {
    return sizeof(this) + txt.length();
  }

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
  String txt;
};
//#endif //USES_C004

//#ifdef USES_C007
/*********************************************************************************************\
* C007_queue_element for queueing requests for C007 Emoncms
\*********************************************************************************************/
class C007_queue_element {
public:

  C007_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}

  C007_queue_element(const struct EventStruct *event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  size_t getSize() const {
    return sizeof(this);
  }

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};
//#endif //USES_C007

/*********************************************************************************************\
* Base class for controllers that only send a single value per request and thus needs to
* keep track of the number of values already sent.
\*********************************************************************************************/
class queue_element_single_value_base {
public:

  queue_element_single_value_base() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}

  queue_element_single_value_base(const struct EventStruct *event, byte value_count) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    valuesSent(0),
    valueCount(value_count) {}

  bool checkDone(bool succesfull) const {
    if (succesfull) { ++valuesSent; }
    return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
  }

  size_t getSize() const {
    size_t total = sizeof(this);

    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int controller_idx;
  byte TaskIndex;
  int idx;
  mutable byte valuesSent; // Value must be set by const function checkDone()
  byte valueCount;
};


//#ifdef USES_C008
/*********************************************************************************************\
* C008_queue_element for queueing requests for 008: Generic HTTP
* Using queue_element_single_value_base
\*********************************************************************************************/
#define C008_queue_element queue_element_single_value_base
//#endif //USES_C008

//#ifdef USES_C009
/*********************************************************************************************\
* C009_queue_element for queueing requests for C009: FHEM HTTP.
\*********************************************************************************************/
class C009_queue_element {
public:

  C009_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}

  C009_queue_element(const struct EventStruct *event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  size_t getSize() const {
    size_t total = sizeof(this);

    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};
//#endif //USES_C009


//#ifdef USES_C010
/*********************************************************************************************\
* C010_queue_element for queueing requests for 010: Generic UDP
* Using queue_element_single_value_base
\*********************************************************************************************/
#define C010_queue_element queue_element_single_value_base
//#endif //USES_C010

//#ifdef USES_C011
/*********************************************************************************************\
* C011_queue_element for queueing requests for 011: Generic HTTP Advanced
\*********************************************************************************************/
#define C011_queue_element simple_queue_element_string_only
//#endif //USES_C011

//#ifdef USES_C012
/*********************************************************************************************\
* C012_queue_element for queueing requests for 012: Blynk
* Using queue_element_single_value_base
\*********************************************************************************************/
#define C012_queue_element queue_element_single_value_base
//#endif //USES_C012

//#ifdef USES_C015
/*********************************************************************************************\
* C015_queue_element for queueing requests for 015: Blynk
* Using queue_element_single_value_base
\*********************************************************************************************/

// #define C015_queue_element queue_element_single_value_base
class C015_queue_element {
public:

  C015_queue_element() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}

  C015_queue_element(const struct EventStruct *event, byte value_count) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    valuesSent(0),
    valueCount(value_count) {}

  bool checkDone(bool succesfull) const {
    if (succesfull) { ++valuesSent; }
    return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
  }

  size_t getSize() const {
    size_t total = sizeof(this);

    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int vPin[VARS_PER_TASK] = {0};
  int controller_idx;
  byte TaskIndex;
  int idx;
  mutable byte valuesSent; // Value must be set by const function checkDone()
  byte valueCount;
};
//#endif //USES_C015

//#ifdef USES_C016
/*********************************************************************************************\
* C016_queue_element for queueing requests for C016: Cached HTTP.
\*********************************************************************************************/
class C016_queue_element {
public:

  C016_queue_element() : timestamp(0), controller_idx(0), TaskIndex(0), sensorType(0) {}

  C016_queue_element(const struct EventStruct *event, byte value_count, unsigned long unixTime) :
    timestamp(unixTime),
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    sensorType(event->sensorType),
    valueCount(value_count)
  {
    const byte BaseVarIndex = TaskIndex * VARS_PER_TASK;

    for (byte i = 0; i < VARS_PER_TASK; ++i) {
      if (i < value_count) {
        values[i] = UserVar[BaseVarIndex + i];
      } else {
        values[i] = 0.0;
      }
    }
  }

  size_t getSize() const {
    return sizeof(this);
  }

  float values[VARS_PER_TASK];
  unsigned long timestamp; // Unix timestamp
  byte controller_idx;
  byte TaskIndex;
  byte sensorType;
  byte valueCount;
};

//#endif //USES_C016

//#ifdef USES_C017
/*********************************************************************************************\
* C017_queue_element for queueing requests for C017: Zabbix Trapper Protocol.
\*********************************************************************************************/
class C017_queue_element {
public:

  C017_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}

  C017_queue_element(const struct EventStruct *event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  size_t getSize() const {
    size_t total = sizeof(this);

    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};
//#endif //USES_C017

//#ifdef USES_C018
/*********************************************************************************************\
* C018_queue_element for queueing requests for C018: TTN/RN2483
\*********************************************************************************************/

#ifdef USES_PACKED_RAW_DATA
String getPackedFromPlugin(struct EventStruct *event, uint8_t sampleSetCount);
#endif // USES_PACKED_RAW_DATA


class C018_queue_element {
public:

  C018_queue_element() {}

  C018_queue_element(struct EventStruct *event, uint8_t sampleSetCount) :
    controller_idx(event->ControllerIndex)
  {
    #ifdef USES_PACKED_RAW_DATA
    packed = getPackedFromPlugin(event, sampleSetCount);
    #endif // USES_PACKED_RAW_DATA
  }

  size_t getSize() const {
    return sizeof(this) + packed.length();
  }

  int controller_idx = 0;
  String packed;
};

//#endif //USES_C018




/*********************************************************************************************\
* ControllerDelayHandlerStruct
\*********************************************************************************************/
template<class T>
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct() :
    lastSend(0),
    minTimeBetweenMessages(CONTROLLER_DELAY_QUEUE_DELAY_DFLT),
    max_queue_depth(CONTROLLER_DELAY_QUEUE_DEPTH_DFLT),
    attempt(0),
    max_retries(CONTROLLER_DELAY_QUEUE_RETRY_DFLT),
    delete_oldest(false),
    must_check_reply(false) {}

  void configureControllerSettings(const ControllerSettingsStruct& settings) {
    minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
    max_queue_depth        = settings.MaxQueueDepth;
    max_retries            = settings.MaxRetry;
    delete_oldest          = settings.DeleteOldest;
    must_check_reply       = settings.MustCheckReply;

    // Set some sound limits when not configured
    if (max_queue_depth == 0) { max_queue_depth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

    if (max_retries == 0) { max_retries = CONTROLLER_DELAY_QUEUE_RETRY_DFLT; }

    if (minTimeBetweenMessages == 0) { minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT; }

    // No less than 10 msec between messages.
    if (minTimeBetweenMessages < 10) { minTimeBetweenMessages = 10; }
  }

  bool queueFull(const T& element) const {
    if (sendQueue.size() >= max_queue_depth) { return true; }

    // Number of elements is not exceeding the limit, check memory
    int freeHeap = ESP.getFreeHeap();

    if (freeHeap > 5000) { return false; // Memory is not an issue.
    }
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = "Controller-";
      log += element.controller_idx + 1;
      log += " : Memory used: ";
      log += getQueueMemorySize();
      log += " bytes ";
      log += sendQueue.size();
      log += " items ";
      log += freeHeap;
      log += " free";
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    return true;
  }

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return false when no item was added.
  bool addToQueue(const T& element) {
    if (delete_oldest) {
      // Force add to the queue.
      // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
      while (queueFull(element)) {
        sendQueue.pop_front();
      }
      sendQueue.emplace_back(element);
      return true;
    }

    if (!queueFull(element)) {
      sendQueue.emplace_back(element);
      return true;
    }
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = get_formatted_Controller_number(element.controller_idx);
      log += " : queue full";
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  // Get the next element.
  // Remove front element when max_retries is reached.
  T* getNext() {
    if (sendQueue.empty()) { return NULL; }

    if (attempt > max_retries) {
      sendQueue.pop_front();
      attempt = 0;

      if (sendQueue.empty()) { return NULL; }
    }
    return &sendQueue.front();
  }

  // Mark as processed and return time to schedule for next process.
  // Return 0 when nothing to process.
  // @param remove_from_queue indicates whether the elements should be removed from the queue.
  unsigned long markProcessed(bool remove_from_queue) {
    if (sendQueue.empty()) { return 0; }

    if (remove_from_queue) {
      sendQueue.pop_front();
      attempt = 0;
    } else {
      ++attempt;
    }
    lastSend = millis();
    return getNextScheduleTime();
  }

  unsigned long getNextScheduleTime() const {
    if (sendQueue.empty()) { return 0; }
    unsigned long nextTime = lastSend + minTimeBetweenMessages;

    if (timePassedSince(nextTime) > 0) {
      nextTime = millis();
    }

    if (nextTime == 0) { nextTime = 1; // Just to make sure it will be executed
    }
    return nextTime;
  }

  size_t getQueueMemorySize() const {
    size_t totalSize = 0;

    for (auto it = sendQueue.begin(); it != sendQueue.end(); ++it) {
      totalSize += it->getSize();
    }
    return totalSize;
  }

  std::list<T>  sendQueue;
  unsigned long lastSend;
  unsigned int  minTimeBetweenMessages;
  byte          max_queue_depth;
  byte          attempt;
  byte          max_retries;
  bool          delete_oldest;
  bool          must_check_reply;
};

ControllerDelayHandlerStruct<MQTT_queue_element> MQTTDelayHandler;

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*


// This macro defines the code needed to create the 'process_c##NNN####M##_delay_queue()'
// function and all needed objects and forward declarations.
// It is a macro to prevent common typo errors.
// This function will perform the (re)scheduling and mark if it is processed (and can be removed)
// The controller itself must implement the 'do_process_c004_delay_queue' function to actually
// send the data.
// Its return value must state whether it can be marked 'Processed'.
// N.B. some controllers only can send one value per iteration, so a returned "false" can mean it
//      was still successful. The controller should keep track of the last value sent
//      in the element stored in the queue.
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN, M)                                                                        \
  ControllerDelayHandlerStruct<C##NNN####M##_queue_element>C##NNN####M##_DelayHandler;                               \
  bool do_process_c##NNN####M##_delay_queue(int controller_number,                                                   \
                                           const C##NNN####M##_queue_element & element,                              \
                                           ControllerSettingsStruct & ControllerSettings);                           \
  void process_c##NNN####M##_delay_queue() {                                                                         \
    C##NNN####M##_queue_element *element(C##NNN####M##_DelayHandler.getNext());                                      \
    if (element == NULL) return;                                                                                     \
    MakeControllerSettings (ControllerSettings);                                                                     \
    LoadControllerSettings(element->controller_idx, ControllerSettings);                                             \
    C##NNN####M##_DelayHandler.configureControllerSettings(ControllerSettings);                                      \
    if (!WiFiConnected(10)) {                                                                                        \
      scheduleNextDelayQueue(TIMER_C##NNN####M##_DELAY_QUEUE, C##NNN####M##_DelayHandler.getNextScheduleTime());     \
      return;                                                                                                        \
    }                                                                                                                \
    START_TIMER;                                                                                                     \
    C##NNN####M##_DelayHandler.markProcessed(do_process_c##NNN####M##_delay_queue(M, *element, ControllerSettings)); \
    STOP_TIMER(C##NNN####M##_DELAY_QUEUE);                                                                           \
    scheduleNextDelayQueue(TIMER_C##NNN####M##_DELAY_QUEUE, C##NNN####M##_DelayHandler.getNextScheduleTime());       \
  }

// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
// If someone knows how to add leading zeros in macros, please be my guest :)
#ifdef USES_C001
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00,  1)
#endif // ifdef USES_C001
#ifdef USES_C003
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00,  3)
#endif // ifdef USES_C003
#ifdef USES_C004
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00,  4)
#endif // ifdef USES_C004
#ifdef USES_C007
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00,  7)
#endif // ifdef USES_C007
#ifdef USES_C008
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 8)
#endif // ifdef USES_C008
#ifdef USES_C009
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 9)
#endif // ifdef USES_C009
#ifdef USES_C010
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0,  10)
#endif // ifdef USES_C010
#ifdef USES_C011
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0,  11)
#endif // ifdef USES_C011
#ifdef USES_C012
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0,  12)
#endif // ifdef USES_C012

/*
 #ifdef USES_C013
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 13)
 #endif
 */

/*
 #ifdef USES_C014
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 14)
 #endif
 */
#ifdef USES_C015
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 15)
#endif // ifdef USES_C015

#ifdef USES_C016
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 16)
#endif // ifdef USES_C016


#ifdef USES_C017
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 17)
#endif // ifdef USES_C017

#ifdef USES_C018
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 18)
#endif


/*
 #ifdef USES_C019
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 19)
 #endif
 */

/*
 #ifdef USES_C020
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 20)
 #endif
 */


// When extending this, also extend in Scheduler.ino:
// void process_interval_timer(unsigned long id, unsigned long lasttimer)

// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

#endif // DELAY_QUEUE_ELEMENTS_H