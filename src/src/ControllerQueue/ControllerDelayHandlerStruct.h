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
  #define CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME 10000
#endif

/*********************************************************************************************\
* ControllerDelayHandlerStruct
\*********************************************************************************************/
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct();

  void configureControllerSettings(const ControllerSettingsStruct& settings);

  bool readyToProcess(const Queue_element_base& element) const;

  bool queueFull(const Queue_element_base& element) const;

  // Return true if message is already present in the queue
  bool isDuplicate(const Queue_element_base& element) const;

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return true when item was added, or skipped as it was considered a duplicate
  bool addToQueue(std::unique_ptr<Queue_element_base> element);

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
  void setAdditionalDelay(unsigned long msecFromNow);

  size_t getQueueMemorySize() const;

  std::list<std::unique_ptr<Queue_element_base>>  sendQueue;
  mutable UnitLastMessageCount_map unitLastMessageCount;
  unsigned long lastSend = 0;
  unsigned int  minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  unsigned long expire_timeout = 0;
  uint8_t       max_queue_depth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
  uint8_t       attempt = 0;
  uint8_t       max_retries = CONTROLLER_DELAY_QUEUE_RETRY_DFLT;
  bool          delete_oldest = false;
  bool          must_check_reply = false;
  bool          deduplicate = false;
  bool          useLocalSystemTime = false;
};



// Uncrustify must not be used on macros, so turn it off.
// Also make sure to wrap the forward declaration of this function in the same wrappers 
// as it may not split the forward declaration into multiple lines.
//
// *INDENT-OFF*



// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
// If someone knows how to add leading zeros in macros, please be my guest :)


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
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN, M)                                                                          \
  extern struct ControllerDelayHandlerStruct *C##NNN##M##_DelayHandler;                                                     \
  bool do_process_c##NNN##M##_delay_queue(int controller_number, const C##NNN##M##_queue_element & element, ControllerSettingsStruct & ControllerSettings); \
  void process_c##NNN##M##_delay_queue();                                                                            \
  bool init_c##NNN##M##_delay_queue(controllerIndex_t ControllerIndex);                                              \
  void exit_c##NNN##M##_delay_queue();                                                                               \

#define DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(NNN, M)                                                                      \
  ControllerDelayHandlerStruct *C##NNN####M##_DelayHandler = nullptr;                                                  \
  void process_c##NNN####M##_delay_queue() {                                                                           \
    if (C##NNN####M##_DelayHandler == nullptr) return;                                                                 \
    C##NNN####M##_queue_element *element(static_cast<C##NNN####M##_queue_element *>(C##NNN####M##_DelayHandler->getNext()));                                       \
    if (element == nullptr) return;                                                                                    \
    if (C##NNN####M##_DelayHandler->readyToProcess(*element)) {                                                        \
      MakeControllerSettings(ControllerSettings);                                                                      \
      if (AllocatedControllerSettings()) {                                                                             \
        LoadControllerSettings(element->controller_idx, ControllerSettings);                                           \
        C##NNN####M##_DelayHandler->configureControllerSettings(ControllerSettings);                                   \
        START_TIMER;                                                                                                   \
        C##NNN####M##_DelayHandler->markProcessed(do_process_c##NNN####M##_delay_queue(M, *element, ControllerSettings)); \
        STOP_TIMER(C##NNN####M##_DELAY_QUEUE);                                                                           \
      }                                                                                                                \
    }                                                                                                                  \
    Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C##NNN####M##_DELAY_QUEUE, C##NNN####M##_DelayHandler->getNextScheduleTime());         \
  }                                                                                                                    \
  bool init_c##NNN####M##_delay_queue(controllerIndex_t ControllerIndex) {                                             \
    if (C##NNN####M##_DelayHandler == nullptr) {                                                                       \
      C##NNN####M##_DelayHandler = new (std::nothrow) (ControllerDelayHandlerStruct);                                  \
    }                                                                                                                  \
    if (C##NNN####M##_DelayHandler == nullptr) { return false; }                                                       \
    MakeControllerSettings(ControllerSettings);                                                                        \
    if (!AllocatedControllerSettings()) {                                                                              \
      return false;                                                                                                    \
    }                                                                                                                  \
    LoadControllerSettings(ControllerIndex, ControllerSettings);                                                       \
    C##NNN####M##_DelayHandler->configureControllerSettings(ControllerSettings);                                       \
    return true;                                                                                                       \
  }                                                                                                                    \
  void exit_c##NNN####M##_delay_queue() {                                                                              \
    if (C##NNN####M##_DelayHandler != nullptr) {                                                                       \
      delete C##NNN####M##_DelayHandler;                                                                               \
      C##NNN####M##_DelayHandler = nullptr;                                                                            \
    }                                                                                                                  \
  }                                                                                                                    \




// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


#endif // CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H