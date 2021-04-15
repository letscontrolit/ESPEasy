#ifndef DATASTRUCTS_TIMER_ID_COUPLE_H
#define DATASTRUCTS_TIMER_ID_COUPLE_H




/*********************************************************************************************\
* TimerHandler Used by the Scheduler
\*********************************************************************************************/

struct timer_id_couple {
  timer_id_couple(unsigned long id, unsigned long newtimer);
  
  timer_id_couple(unsigned long id);

  bool operator<(const timer_id_couple& other);

  unsigned long _id;
  unsigned long _timer;
};


#endif // DATASTRUCTS_TIMER_ID_COUPLE_H