#include "../Helpers/msecTimerHandlerStruct.h"


#include "../Helpers/ESPEasy_time_calc.h"


#define MAX_SCHEDULER_WAIT_TIME 20 // Max delay used in the scheduler for passing idle time.

  msecTimerHandlerStruct::msecTimerHandlerStruct() : eco_mode(true)
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

  void msecTimerHandlerStruct::remove(unsigned long id) {
    timer_id_couple item(id, 0);
    remove(item);
  }

  // Check if timeout has been reached and also return its set timer.
  // Return 0 if no item has reached timeout moment.
  unsigned long msecTimerHandlerStruct::getNextId(unsigned long& timer) {
#ifndef BUILD_NO_DEBUG
    ++get_called;
#endif

    if (_timer_ids.empty()) {
      recordIdle();
      delay(eco_mode ? MAX_SCHEDULER_WAIT_TIME : 0); // Nothing to do, try save some power.
      return 0;
    }
    const timer_id_couple item = _timer_ids.front();
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

    _timer_ids.pop_front();
    timer = item._timer;
#ifndef BUILD_NO_DEBUG
    ++get_called_ret_id;
#endif
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
#ifndef BUILD_NO_DEBUG
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
  #endif

  void msecTimerHandlerStruct::updateIdleTimeStats() {
    const long duration = timePassedSince(last_log_start_time) * 10;
    if (duration <= 1000) {
      // No need to recompute it over a small interval
      // Also makes sure duration != 0 as it is used in division
      return;
    }
    recordRunning(); // Make sure currently active 'idle' time is included
    last_log_start_time  = millis();
    idle_time_pct = static_cast<float>(total_idle_time_usec);
    idle_time_pct /= static_cast<float>(duration);
    total_idle_time_usec = 0;
  }

  float msecTimerHandlerStruct::getIdleTimePct()  {
    updateIdleTimeStats();
    return idle_time_pct;
  }

  void msecTimerHandlerStruct::insert(const timer_id_couple& item) {
    if (item._id == 0) { return; }

    // Make sure only one is present with the same id.
    // Keep in mind: order is based on timer, uniqueness is based on id.
    _timer_ids.remove_if(item);

    // Insert into a sorted list, so find first pos which should be handled after this item.
    auto prev = _timer_ids.begin();
    if (_timer_ids.empty() || prev == _timer_ids.end() || item < *prev) {
      _timer_ids.push_front(item);
      return;
    }

//    _timer_ids.push_front(item);
//    _timer_ids.sort();
//    return;


    auto it = prev;
    ++it;
    for (;it != _timer_ids.end() && *it < item; ++it, ++prev) {}


//    auto stats_it = 
    _timer_ids.insert_after(prev, item);
#ifndef BUILD_NO_DEBUG
    auto size = std::distance(_timer_ids.begin(), prev) + 1;

    // TODO TD-er: No need to loop each time through the list.
    // Stats are just some indication, don't need to be that exact.
    // std::forward_list doesn't have size()
//    for (; stats_it != _timer_ids.end(); ++stats_it, ++size) {}
    if (size > max_queue_length) { max_queue_length = size; }    
#endif
  }

  void msecTimerHandlerStruct::remove(const timer_id_couple& item) {
    if (item._id == 0) { return; }

    // Make sure only one is present with the same id.
    _timer_ids.remove_if(item);
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
    total_idle_time_usec += usecPassedSince_fast(last_exec_time_usec);
  }
