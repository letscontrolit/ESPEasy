#ifndef CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H
#define CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H

#include "../../ESPEasy_common.h"

#include "../ControllerQueue/Queue_element_base.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/CPlugins.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/Protocol.h"
#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"

#include <Arduino.h>
#include <list>
#include <memory> // For std::shared_ptr
#include <new>    // std::nothrow

#ifndef CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME
  # define CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME 10000
#endif // ifndef CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME

typedef bool (*do_process_function)(int,
                                    const Queue_element_base&,
                                    ControllerSettingsStruct&);

/*********************************************************************************************\
* ControllerDelayHandlerStruct
\*********************************************************************************************/
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct();

  bool configureControllerSettings(controllerIndex_t ControllerIndex);
  void configureControllerSettings(const ControllerSettingsStruct& settings);

  bool readyToProcess(const Queue_element_base& element) const;

  bool queueFull(controllerIndex_t controller_idx) const;

  // Return true if message is already present in the queue
  bool isDuplicate(const Queue_element_base& element) const;

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return true when item was added, or skipped as it was considered a duplicate
  bool addToQueue(std::unique_ptr<Queue_element_base>element);

  // Get the next element.
  // Remove front element when max_retries is reached.
  Queue_element_base* getNext();

  // Mark as processed and return time to schedule for next process.
  // Return 0 when nothing to process.
  // @param remove_from_queue indicates whether the elements should be removed from the queue.
  unsigned long markProcessed(bool remove_from_queue);

  unsigned long getNextScheduleTime() const;

  // Set the "lastSend" to "now" + some additional delay.
  // This will cause the next schedule time to be delayed to
  // msecFromNow + minTimeBetweenMessages
  void   setAdditionalDelay(unsigned long msecFromNow);

  size_t getQueueMemorySize() const;

  void   process(
    int                                controller_number,
    do_process_function                func,
    TimingStatsElements                timerstats_id,
    ESPEasy_Scheduler::IntervalTimer_e timerID);

  std::list<std::unique_ptr<Queue_element_base> >sendQueue;
  mutable UnitLastMessageCount_map               unitLastMessageCount;
  unsigned long                                  lastSend               = 0;
  unsigned int                                   minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  unsigned long                                  expire_timeout         = 0;
  uint8_t                                        max_queue_depth        = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
  uint8_t                                        attempt                = 0;
  uint8_t                                        max_retries            = CONTROLLER_DELAY_QUEUE_RETRY_DFLT;
  bool                                           delete_oldest          = false;
  bool                                           must_check_reply       = false;
  bool                                           deduplicate            = false;
  bool                                           useLocalSystemTime     = false;
};


#endif // CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H
