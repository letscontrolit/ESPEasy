#ifndef HELPERS_MSECTIMERHANDLERSTRUCT_H
#define HELPERS_MSECTIMERHANDLERSTRUCT_H


#include <Arduino.h>
#include <list>

#include "../DataStructs/timer_id_couple.h"


struct msecTimerHandlerStruct {
  msecTimerHandlerStruct();

  void setEcoMode(bool enabled);

  void registerAt(unsigned long id,
                  unsigned long timer);

  void remove(unsigned long id);

  // Check if timeout has been reached and also return its set timer.
  // Return 0 if no item has reached timeout moment.
  unsigned long getNextId(unsigned long& timer);

  // Check if a give ID is scheduled and if so, return the set timer.
  // N.B. the ID is the mixed ID.
  bool   getTimerForId(unsigned long  id,
                       unsigned long& timer) const;

  String getQueueStats();

  void   updateIdleTimeStats();

  float  getIdleTimePct() const;

private:

  void insert(const timer_id_couple& item);

  void remove(const timer_id_couple& item);

  void recordIdle();

  void recordRunning();

  // Statistics
  unsigned long get_called;
  unsigned long get_called_ret_id;
  unsigned long max_queue_length;

  // Compute idle system time
  uint64_t last_exec_time_usec;
  uint64_t total_idle_time_usec;
  uint32_t last_log_start_time;
  float         idle_time_pct;
  bool          is_idle;
  bool          eco_mode;

  // The list of set timers
  std::list<timer_id_couple>_timer_ids;
};

#endif // HELPERS_MSECTIMERHANDLERSTRUCT_H
