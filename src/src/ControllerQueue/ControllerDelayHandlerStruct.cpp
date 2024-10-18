#include "../ControllerQueue/ControllerDelayHandlerStruct.h"


ControllerDelayHandlerStruct::ControllerDelayHandlerStruct() :
  lastSend(0),
  minTimeBetweenMessages(CONTROLLER_DELAY_QUEUE_DELAY_DFLT),
  expire_timeout(0),
  max_queue_depth(CONTROLLER_DELAY_QUEUE_DEPTH_DFLT),
  attempt(0),
  max_retries(CONTROLLER_DELAY_QUEUE_RETRY_DFLT),
  delete_oldest(false),
  must_check_reply(false),
  deduplicate(false),
  useLocalSystemTime(false) {}

bool ControllerDelayHandlerStruct::cacheControllerSettings(controllerIndex_t ControllerIndex)
{
  MakeControllerSettings(ControllerSettings);

  if (!AllocatedControllerSettings()) {
    return false;
  }
  LoadControllerSettings(ControllerIndex, *ControllerSettings);
  cacheControllerSettings(*ControllerSettings);
  return true;
}

void ControllerDelayHandlerStruct::cacheControllerSettings(const ControllerSettingsStruct& settings) {
  minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
  max_queue_depth        = settings.MaxQueueDepth;
  max_retries            = settings.MaxRetry;
  delete_oldest          = settings.DeleteOldest;
  must_check_reply       = settings.MustCheckReply;
  deduplicate            = settings.deduplicate();
  useLocalSystemTime     = settings.useLocalSystemTime();
#ifdef USES_ESPEASY_NOW
  enableESPEasyNowFallback = settings.enableESPEasyNowFallback();
#endif

  if (settings.allowExpire()) {
    expire_timeout = max_queue_depth * max_retries * (minTimeBetweenMessages + settings.ClientTimeout);

    if (expire_timeout < CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME) {
      expire_timeout = CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME;
    }
  } else {
    expire_timeout = 0;
  }

  // Set some sound limits when not configured
  if (max_queue_depth == 0) { max_queue_depth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

  if (max_retries == 0) { max_retries = CONTROLLER_DELAY_QUEUE_RETRY_DFLT; }

  if (minTimeBetweenMessages == 0) { minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT; }

  // No less than 10 msec between messages.
  if (minTimeBetweenMessages < 10) { minTimeBetweenMessages = 10; }
}

bool ControllerDelayHandlerStruct::readyToProcess(const Queue_element_base& element) const {
  const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(element._controller_idx);

  if (protocolIndex == INVALID_PROTOCOL_INDEX) {
    return false;
  }

  if (!enableESPEasyNowFallback && getProtocolStruct(protocolIndex).needsNetwork) {
    return NetworkConnected(10);
  }
  return true;
}

bool ControllerDelayHandlerStruct::queueFull(controllerIndex_t controller_idx) const {
  if (sendQueue.size() >= max_queue_depth) { 
    return true; 
  }

  // Number of elements is not exceeding the limit, check memory
  int freeHeap = FreeMem();
  {
    /*
      #ifdef USE_SECOND_HEAP
    const int freeHeap2 = FreeMem2ndHeap();

    if (freeHeap2 < freeHeap) {
      freeHeap = freeHeap2;
    }
      #endif // ifdef USE_SECOND_HEAP
      */
  }

#ifdef ESP32
  if (freeHeap > 50000) 
#else
  if (freeHeap > 5000) 
#endif
  {
    return false; // Memory is not an issue.
  }
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Controller-");
    log += controller_idx + 1;
    log += F(" : Memory used: ");
    log += getQueueMemorySize();
    log += F(" bytes ");
    log += sendQueue.size();
    log += F(" items ");
    log += freeHeap;
    log += F(" free");
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return true;
}

// Return true if message is already present in the queue
bool ControllerDelayHandlerStruct::isDuplicate(const Queue_element_base& element) const {
  // Some controllers may receive duplicate messages, due to lost acknowledgement
  // This is actually the same message, so this should not be processed.
  if (!unitMessageRouteInfo_map.isNew(element.getMessageRouteInfo())) {
    return true;
  }

  // The unit message count is still stored to make sure a new one with the same count
  // is considered a duplicate, even when the queue is empty.
  unitMessageRouteInfo_map.add(element.getMessageRouteInfo());

  // the setting 'deduplicate' does look at the content of the message and only compares it to messages in the queue.
  if (deduplicate && !sendQueue.empty()) {
    // Use reverse iterator here, as it is more likely a duplicate is added shortly after another.
    auto it = sendQueue.rbegin(); // Same as back()

    for (; it != sendQueue.rend(); ++it) {
      if (element.isDuplicate(*(it->get()))) {
#ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          const cpluginID_t cpluginID = getCPluginID_from_ControllerIndex(it->get()->_controller_idx);
          addLogMove(LOG_LEVEL_DEBUG, concat(get_formatted_Controller_number(cpluginID), F(" : Remove duplicate")));
        }
#endif // ifndef BUILD_NO_DEBUG
        return true;
      }
    }
  }
  return false;
}

// Try to add to the queue, if permitted by "delete_oldest"
// Return true when item was added, or skipped as it was considered a duplicate
bool ControllerDelayHandlerStruct::addToQueue(std::unique_ptr<Queue_element_base>element) {
  if (!element) { 
    return false;
  }
  if (isDuplicate(*element)) {
    return true;
  }

  if (delete_oldest) {
    // Force add to the queue.
    // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
    while (queueFull(element->_controller_idx)) {
      sendQueue.pop_front();
      attempt = 0;
    }
  }

  if (!queueFull(element->_controller_idx)) {
    #ifdef USE_SECOND_HEAP
    // Do not store in 2nd heap, std::list cannot handle 2nd heap well
    HeapSelectDram ephemeral;
    #endif // ifdef USE_SECOND_HEAP

    sendQueue.push_back(std::move(element));

    return true;
  }
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    const cpluginID_t cpluginID = getCPluginID_from_ControllerIndex((*element)._controller_idx);
    addLogMove(LOG_LEVEL_DEBUG, concat(get_formatted_Controller_number(cpluginID), F(" : queue full")));
  }
#endif // ifndef BUILD_NO_DEBUG
  return false;
}

// Get the next element.
// Remove front element when max_retries is reached.
Queue_element_base * ControllerDelayHandlerStruct::getNext() {
  if (sendQueue.empty()) { return nullptr; }

  if (attempt > max_retries) {
    sendQueue.pop_front();
    attempt = 0;
  }

  if (expire_timeout != 0) {
    bool done = false;

    while (!done && !sendQueue.empty()) {
      if ((sendQueue.front().get() != nullptr) && (timePassedSince(sendQueue.front()->_timestamp) < static_cast<long>(expire_timeout))) {
        done = true;
      } else {
        sendQueue.pop_front();
        attempt = 0;
      }
    }
  }

  if (sendQueue.empty()) { return nullptr; }
  return sendQueue.front().get();
}

// Mark as processed and return time to schedule for next process.
// Return 0 when nothing to process.
// @param remove_from_queue indicates whether the elements should be removed from the queue.
unsigned long ControllerDelayHandlerStruct::markProcessed(bool remove_from_queue) {
  if (sendQueue.empty()) { return 0; }

  if (remove_from_queue) {
    sendQueue.pop_front();
    attempt  = 0;
    lastSend = millis();
  } else {
    ++attempt;
  }
  return getNextScheduleTime();
}

unsigned long ControllerDelayHandlerStruct::getNextScheduleTime() const {
  if (sendQueue.empty()) { return 0; }
  unsigned long nextTime = lastSend + minTimeBetweenMessages;

  if (timePassedSince(nextTime) > 0) {
    nextTime = millis();
  }

  if (nextTime == 0) { nextTime = 1; // Just to make sure it will be executed
  }
  return nextTime;
}

// Set the "lastSend" to "now" + some additional delay.
// This will cause the next schedule time to be delayed to
// msecFromNow + minTimeBetweenMessages
void ControllerDelayHandlerStruct::setAdditionalDelay(unsigned long msecFromNow) {
  lastSend = millis() + msecFromNow;
}

size_t ControllerDelayHandlerStruct::getQueueMemorySize() const {
  size_t totalSize = 0;

  for (auto it = sendQueue.begin(); it != sendQueue.end(); ++it) {
    if (it->get() != nullptr) {
      totalSize += it->get()->getSize();
    }
  }
  return totalSize;
}

void ControllerDelayHandlerStruct::process(
  cpluginID_t                        cpluginID,
  do_process_function                func,
  TimingStatsElements                timerstats_id,
  SchedulerIntervalTimer_e timerID) 
{
  Queue_element_base *element(static_cast<Queue_element_base *>(getNext()));

  if (element == nullptr) { return; }

  if (enableESPEasyNowFallback || readyToProcess(*element)) {
    MakeControllerSettings(ControllerSettings);

    if (AllocatedControllerSettings()) {
      LoadControllerSettings(element->_controller_idx, *ControllerSettings);
      cacheControllerSettings(*ControllerSettings);
      START_TIMER;
      markProcessed(func(cpluginID, *element, *ControllerSettings));
      #if FEATURE_TIMING_STATS
      STOP_TIMER_VAR(timerstats_id);
      #endif
    }
  }
  Scheduler.scheduleNextDelayQueue(timerID, getNextScheduleTime());
}
