#ifndef CPLUGIN_HELPER_H
#define CPLUGIN_HELPER_H CPLUGIN_HELPER_H

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

  int controller_idx;
  String _topic;
  String _payload;
  boolean _retained;
};

/*********************************************************************************************\
 * C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
class C001_queue_element {
public:
  C001_queue_element() : controller_idx(0) {}
  C001_queue_element(int ctrl_idx, const String& req) : controller_idx(ctrl_idx), request(req) {}

  int controller_idx;
  String request;
};

/*********************************************************************************************\
 * C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
class C003_queue_element {
public:
  C003_queue_element() : controller_idx(0) {}
  C003_queue_element(int ctrl_idx, const String& req) : controller_idx(ctrl_idx), url(req) {}

  int controller_idx;
  String url;
};

/*********************************************************************************************\
 * C004_queue_element for queueing requests for C004 ThingSpeak.
 *   Typical use case for Thingspeak is to only send values every N seconds/minutes.
 *   So we just store everything needed to recreate the event when the time is ready.
\*********************************************************************************************/
class C004_queue_element {
public:
  C004_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}
  C004_queue_element(const struct EventStruct* event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};

/*********************************************************************************************\
 * C007_queue_element for queueing requests for C007 Emoncms
\*********************************************************************************************/
class C007_queue_element {
public:
  C007_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}
  C007_queue_element(const struct EventStruct* event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};


/*********************************************************************************************\
 * ControllerDelayHandlerStruct
\*********************************************************************************************/
template<class T>
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct() :
      lastSend(0), minTimeBetweenMessages(100), max_buffer(10), attempt(0), max_attempt(10), delete_oldest(false) {}

  void configureControllerSettings(const ControllerSettingsStruct& settings) {
    minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
    max_buffer = settings.MaxBufferDepth;
    max_attempt = settings.MaxRetry;
    delete_oldest = settings.DeleteOldest;
    // Set some sound limits
    if (max_buffer == 0) max_buffer = 10;
    if (max_attempt == 0) max_attempt = 10;
    if (minTimeBetweenMessages == 0) minTimeBetweenMessages = 100;
    if (minTimeBetweenMessages < 10) minTimeBetweenMessages = 10;
  }

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return false when the buffer was full. Success depends on "delete_oldest"
  bool addToQueue(const T& element) {
    if (delete_oldest) {
      return forceAddToQueue(element);
    }
    if (sendQueue.size() < max_buffer) {
      sendQueue.emplace_back(element);
      return true;
    }
    return false;
  }

  // Force add to the queue.
  // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
  // Return true when no elements removed from queue.
  bool forceAddToQueue(const T& element) {
    sendQueue.emplace_back(element);
    if (sendQueue.size() <= max_buffer) {
      return true;
    }
    sendQueue.pop_front();
    return false;
  }

  // Get the next element.
  // Remove front element when max_attempt is reached.
  bool getNext(T& element) {
    if (sendQueue.empty()) return false;
    if (max_attempt <= attempt) {
      sendQueue.pop_front();
      attempt = 0;
    }
    element = sendQueue.front();
    return true;
  }

  // Mark as processed and return time to schedule for next process.
  // Return 0 when nothing to process.
  // @param remove_from_queue indicates whether the elements should be removed from the queue.
  unsigned long markProcessed(bool remove_from_queue) {
    if (sendQueue.empty()) return 0;
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
    if (sendQueue.empty()) return 0;
    unsigned long nextTime = lastSend + minTimeBetweenMessages;
    if (timePassedSince(nextTime) > 0) {
      nextTime = millis();
    }
    if (nextTime == 0) nextTime = 1; // Just to make sure it will be executed
    return nextTime;
  }

  std::list<T> sendQueue;
  unsigned long lastSend;
  unsigned int minTimeBetweenMessages;
  byte max_buffer;
  byte attempt;
  byte max_attempt;
  bool delete_oldest;
};

ControllerDelayHandlerStruct<MQTT_queue_element> MQTTDelayHandler;

// Forward declaration
void scheduleNextDelayQueue(unsigned long id, unsigned long nextTime);
String LoadControllerSettings(int ControllerIndex, byte* memAddress, int datasize);

// This macro defines the code needed to create the 'process_c##NNN##_delay_queue()'
// function and all needed objects and forward declarations.
// It is a macro to prevent common typo errors.
// This function will perform the (re)scheduling and mark if it is processed (and can be removed)
// The controller itself must implement the 'do_process_c004_delay_queue' function to actually
// send the data.
// Its return value must state whether it can be marked 'Processed'.
// N.B. some controllers only can send one value per iteration, so a returned "false" can mean it
//      was still successful. The controller should keep track of the last value sent
//      in the element stored in the queue.
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN) \
                ControllerDelayHandlerStruct<C##NNN##_queue_element> C##NNN##_DelayHandler; \
                bool do_process_c##NNN##_delay_queue(const C##NNN##_queue_element& element, ControllerSettingsStruct& ControllerSettings); \
                void process_c##NNN##_delay_queue() { \
                  C##NNN##_queue_element element; \
                  if (!C##NNN##_DelayHandler.getNext(element)) return; \
                  ControllerSettingsStruct ControllerSettings; \
                  LoadControllerSettings(element.controller_idx, (byte*)&ControllerSettings, sizeof(ControllerSettings)); \
                  C##NNN##_DelayHandler.configureControllerSettings(ControllerSettings); \
                  if (!WiFiConnected(100)) { \
                    scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                    return; \
                  } \
                  C##NNN##_DelayHandler.markProcessed(do_process_c##NNN##_delay_queue(element, ControllerSettings)); \
                  scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                }

// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
#ifdef USES_C001
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(001)
#endif
#ifdef USES_C003
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(003)
#endif
#ifdef USES_C004
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(004)
#endif
#ifdef USES_C007
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(007)
#endif
/*
#ifdef USES_C008
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(008)
#endif
#ifdef USES_C009
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(009)
#endif
#ifdef USES_C010
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(010)
#endif
#ifdef USES_C011
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(011)
#endif
#ifdef USES_C012
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(012)
#endif
#ifdef USES_C013
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(013)
#endif
*/




#endif // CPLUGIN_HELPER_H
