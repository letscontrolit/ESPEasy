#ifndef CPLUGIN_HELPER_H
#define CPLUGIN_HELPER_H CPLUGIN_HELPER_H

// These element classes should be defined as class, to be used as template.

/*********************************************************************************************\
 * MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element {
public:
  MQTT_queue_element() : _controller_idx(0), _retained(false) {}

  MQTT_queue_element(int controller_idx,
    const String& topic, const String& payload, boolean retained) :
    _controller_idx(controller_idx), _topic(topic), _payload(payload), _retained(retained)
     {}

  int _controller_idx;
  String _topic;
  String _payload;
  boolean _retained;
};

/*********************************************************************************************\
 * C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
class C001_queue_element {
public:
  C001_queue_element() : _controller_idx(0) {}
  C001_queue_element(int controller_idx, const String& req) : _controller_idx(controller_idx), request(req) {}

  int _controller_idx;
  String request;
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
ControllerDelayHandlerStruct<C001_queue_element> C001_DelayHandler;




#endif // CPLUGIN_HELPER_H
