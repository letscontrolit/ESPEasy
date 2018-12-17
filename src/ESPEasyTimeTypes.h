#ifndef ESPEASY_TIMETYPES_H_
#define ESPEASY_TIMETYPES_H_

#include <stdint.h>
#include <list>
#include <time.h>

// convenient constants for TimeChangeRules
enum week_t {Last=0, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
// For Daylight Saving Time Around the World, see:
// - https://www.timeanddate.com/time/dst/2018.html
// - https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
struct TimeChangeRule {
    TimeChangeRule() :  week(0), dow(1), month(1), hour(0), offset(0) {}
    TimeChangeRule(uint8_t weeknr, uint8_t downr, uint8_t m, uint8_t h, uint16_t minutesoffset) :
        week(weeknr), dow(downr), month(m), hour(h), offset(minutesoffset) {}

    // Construct time change rule from stored values optimized for minimum space.
    TimeChangeRule(uint16_t flash_stored_value, int16_t minutesoffset) : offset(minutesoffset) {
      hour = flash_stored_value & 0x001f;
      month = (flash_stored_value >> 5) & 0x000f;
      dow = (flash_stored_value >> 9) & 0x0007;
      week = (flash_stored_value >> 12) & 0x0007;
    }

    uint16_t toFlashStoredValue() const {
      uint16_t value = hour;
      value = value | (month << 5);
      value = value | (dow << 9);
      value = value | (week << 12);
      return value;
    }

    bool isValid() const {
      return (week <= 4) && (dow != 0) && (dow <= 7) &&
             (month != 0) && (month <= 12) && (hour <= 23) &&
             (offset > -720) && (offset < 900); // UTC-12h ... UTC+14h + 1h DSToffset
    }

    uint8_t week;      // First, Second, Third, Fourth, or Last week of the month
    uint8_t dow;       // day of week, 1=Sun, 2=Mon, ... 7=Sat
    uint8_t month;     // 1=Jan, 2=Feb, ... 12=Dec
    uint8_t hour;      // 0-23
    int16_t offset;    // offset from UTC in minutes
};

// Forward declartions
void applyTimeZone(uint32_t curTime = 0);
void setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime = 0);
uint32_t calcTimeChangeForRule(const TimeChangeRule& r, int yr);
String getTimeString(char delimiter, bool show_seconds=true);
String getTimeString_ampm(char delimiter, bool show_seconds=true);
long timeDiff(unsigned long prev, unsigned long next);
long timePassedSince(unsigned long timestamp);
boolean timeOutReached(unsigned long timer);
long usecPassedSince(unsigned long timestamp);
boolean usecTimeOutReached(unsigned long timer);
void setPluginTaskTimer(unsigned long msecFromNow, byte plugin, short taskIndex, int Par1,
  int Par2 = 0, int Par3 = 0, int Par4 = 0, int Par5 = 0);
void setGPIOTimer(unsigned long msecFromNow, int Par1,
  int Par2 = 0, int Par3 = 0, int Par4 = 0, int Par5 = 0);



/*********************************************************************************************\
 * TimerHandler Used by the Scheduler
\*********************************************************************************************/

struct timer_id_couple {
  timer_id_couple(unsigned long id, unsigned long newtimer) : _id(id), _timer(newtimer) {}

  timer_id_couple(unsigned long id) : _id(id) {
    _timer = millis();
  }

  bool operator<(const timer_id_couple& other) {
    const unsigned long now(millis());
    // timediff > 0, means timer has already passed
    return (timeDiff(_timer, now) > timeDiff(other._timer, now));
  }

  unsigned long _id;
  unsigned long _timer;
};

struct msecTimerHandlerStruct {

  msecTimerHandlerStruct() : get_called(0), get_called_ret_id(0), max_queue_length(0),
      last_exec_time_usec(0), total_idle_time_usec(0), is_idle(false), idle_time_pct(0.0)
  {
    last_log_start_time = millis();
  }

  void registerAt(unsigned long id, unsigned long timer) {
    timer_id_couple item(id, timer);
    insert(item);
  }

  // Check if timeout has been reached and also return its set timer.
  // Return 0 if no item has reached timeout moment.
  unsigned long getNextId(unsigned long& timer) {
    ++get_called;
    if (_timer_ids.empty()) {
      recordIdle();
      return 0;
    }
    timer_id_couple item = _timer_ids.front();
    if (!timeOutReached(item._timer)) {
      recordIdle();
      return 0;
    }
    recordRunning();
    unsigned long size = _timer_ids.size();
    if (size > max_queue_length) max_queue_length = size;
    _timer_ids.pop_front();
    timer = item._timer;
    ++get_called_ret_id;
    return item._id;
  }

  String getQueueStats() {

    String result;
    result += get_called;
    result += '/';
    result += get_called_ret_id;
    result += '/';
    result += max_queue_length;
    result += '/';
    result += idle_time_pct;
    get_called = 0;
    get_called_ret_id = 0;
    //max_queue_length = 0;
    return result;
  }

  void updateIdleTimeStats() {
    const long duration = timePassedSince(last_log_start_time);
    last_log_start_time = millis();
    idle_time_pct = total_idle_time_usec / duration / 10.0;
    total_idle_time_usec = 0;
  }

  float getIdleTimePct() {
    return idle_time_pct;
  }

private:
  struct match_id {
    match_id(unsigned long id) : _id(id) {}
    bool operator() (const timer_id_couple& item) { return _id == item._id; }
    unsigned long _id;
  };

  void insert(const timer_id_couple& item) {
    if (item._id == 0) return;

    // Make sure only one is present with the same id.
    _timer_ids.remove_if(match_id(item._id));
    const bool mustSort = !_timer_ids.empty();
    _timer_ids.push_front(item);
    if (mustSort)
      _timer_ids.sort(); // TD-er: Must check if this is an expensive operation.
    // It should be a relative light operation, to insert into a sorted list.
    // Perhaps it is better to use std::set ????
    // Keep in mind: order is based on timer, uniqueness is based on id.
  }

  void recordIdle() {
    if (is_idle) return;
    last_exec_time_usec = micros();
    is_idle = true;
    delay(0); // Nothing to do, so leave time for backgroundtasks
  }

  void recordRunning() {
    if (!is_idle) return;
    is_idle = false;
    total_idle_time_usec += usecPassedSince(last_exec_time_usec);
  }

  // Statistics
  unsigned long get_called;
  unsigned long get_called_ret_id;
  unsigned long max_queue_length;

  // Compute idle system time
  unsigned long last_exec_time_usec;
  unsigned long total_idle_time_usec;
  unsigned long last_log_start_time;
  bool is_idle;
  float idle_time_pct;

  // The list of set timers
  std::list<timer_id_couple> _timer_ids;
};



#endif /* ESPEASY_TIMETYPES_H_ */
