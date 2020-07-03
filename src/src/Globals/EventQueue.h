#ifndef GLOBALS_EVENTQUEUE_H
#define GLOBALS_EVENTQUEUE_H

#include "../DataStructs/EventStructCommandWrapper.h"
#include "../DataStructs/EventQueue.h"

#include <list>

extern std::list<EventStructCommandWrapper> ScheduledEventQueue;
extern EventQueueStruct eventQueue;

#endif // GLOBALS_EVENTQUEUE_H