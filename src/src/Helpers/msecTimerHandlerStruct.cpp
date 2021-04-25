#include "msecTimerHandlerStruct.h"

#include <Arduino.h>

#include "ESPEasy_time_calc.h"


#define MAX_SCHEDULER_WAIT_TIME 5 // Max delay used in the scheduler for passing idle time.

  msecTimerHandlerStruct::msecTimerHandlerStruct() : get_called(0), get_called_ret_id(0), max_queue_length(0),
    last_exec_time_usec(0), total_idle_time_usec(0),  idle_time_pct(0.0f), is_idle(false), eco_mode(true)
  {
    last_log_start_time = millis();
  }

  void msecTimerHandlerStruct::setEcoMode(bool enabled) {
    eco_mode = enabled;
  }

  void msecTimerHandlerStruct::registerAt(unsigned long id, unsigned long timer) {
    timer_id_couple item(id, timer);

    insert(item);
  }

  // Check if timeout has been reached and also return its set timer.
  // Return 0 if no item has reached timeout moment.
  unsigned long msecTimerHandlerStruct::getNextId(unsigned long& timer) {
    ++get_called;

    if (_timer_ids.empty()) {
      recordIdle();

      if (eco_mode) {
        delay(MAX_SCHEDULER_WAIT_TIME); // Nothing to do, try save some power.
      }
      return 0;
    }
    timer_id_couple item = _timer_ids.front();
    const long passed    = timePassedSince(item._timer);

    if (passed < 0) {
      // No timeOutReached
      recordIdle();

      if (eco_mode) {
        long waitTime = (-1 * passed) - 1; // will be non negative

        if (waitTime > MAX_SCHEDULER_WAIT_TIME) {
          waitTime = MAX_SCHEDULER_WAIT_TIME;
        } else if (waitTime < 0) {  //-V547
          // Should not happen, but just to be sure we will not wait forever.
          waitTime = 0;
        }
        delay(waitTime);
      }
      return 0;
    }
    recordRunning();
    unsigned long size = _timer_ids.size();

    if (size > max_queue_length) { max_queue_length = size; }
    _timer_ids.pop_front();
    timer = item._timer;
    ++get_called_ret_id;
    return item._id;
  }


  bool msecTimerHandlerStruct::getTimerForId(unsigned long id, unsigned long& timer) const {
    for (auto it = _timer_ids.begin(); it != _timer_ids.end(); ++it) {
      if (it->_id == id) {
        timer = it->_timer;
        return true;
      }
    }
    return false;
  }

  String msecTimerHandlerStruct::getQueueStats() {
    String result;

    result           += get_called;
    result           += '/';
    result           += get_called_ret_id;
    result           += '/';
    result           += max_queue_length;
    result           += '/';
    result           += idle_time_pct;
    get_called        = 0;
    get_called_ret_id = 0;

    // max_queue_length = 0;
    return result;
  }

  void msecTimerHandlerStruct::updateIdleTimeStats() {
    const long duration = timePassedSince(last_log_start_time);

    last_log_start_time  = millis();
    idle_time_pct        = static_cast<float>(total_idle_time_usec) / duration / 10.0f;
    total_idle_time_usec = 0;
  }

  float msecTimerHandlerStruct::getIdleTimePct() const {
    return idle_time_pct;
  }

  struct match_id {
    match_id(unsigned long id) : _id(id) {}

    bool operator()(const timer_id_couple& item) {
      return _id == item._id;
    }

    unsigned long _id;
  };

  void msecTimerHandlerStruct::insert(const timer_id_couple& item) {
    if (item._id == 0) { return; }

    // Make sure only one is present with the same id.
    _timer_ids.remove_if(match_id(item._id));
    const bool mustSort = !_timer_ids.empty();
    _timer_ids.push_front(item);

    if (mustSort) {
      _timer_ids.sort(); // TD-er: Must check if this is an expensive operation.
    }

    // It should be a relative light operation, to insert into a sorted list.
    // Perhaps it is better to use std::set ????
    // Keep in mind: order is based on timer, uniqueness is based on id.
  }

  void msecTimerHandlerStruct::recordIdle() {
    if (is_idle) { return; }
    last_exec_time_usec = micros();
    is_idle             = true;
    delay(0); // Nothing to do, so leave time for backgroundtasks
  }

  void msecTimerHandlerStruct::recordRunning() {
    if (!is_idle) { return; }
    is_idle               = false;
    total_idle_time_usec += usecPassedSince(last_exec_time_usec);
  }
