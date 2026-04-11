
#include "../PluginStructs/P089_ping_service_struct.h"

#ifdef ESP32

P089_ping_service_struct*P089_ping_service = nullptr;

/**
 * Ping execute handler task
 */
void P089_execute_ping_task(void *parameter)
{
  P089_ping_task_data*ping_task_data = static_cast<P089_ping_task_data *>(parameter);

  if ((ping_task_data->status == P089_task_status::Working) && (nullptr != ping_task_data->espPing)) {
    // Blocking operation
    ping_task_data->result = ping_task_data->espPing->ping(ping_task_data->ip, ping_task_data->count);

    // Results are in
    ping_task_data->avgTime = ping_task_data->espPing->averageTime();
    ping_task_data->status  = P089_task_status::Ready;
  }

  ping_task_data->taskHandle = NULL;
  vTaskDelete(ping_task_data->taskHandle);
}

/**
 * P089_ping_service_struct
 */
P089_ping_service_struct::P089_ping_service_struct() {
  espPing = new (std::nothrow) PingClass();
}

P089_ping_service_struct::~P089_ping_service_struct() {
  if (_ping_task_data.taskHandle) {
    vTaskDelete(_ping_task_data.taskHandle);
    _ping_task_data.taskHandle = NULL;
  }
  _requests.clear();
  delete espPing;
  espPing = nullptr;
}

bool P089_ping_service_struct::addPingRequest(taskIndex_t       TaskIndex,
                                              P089_ping_request request) {
  bool result = false; // Something is not ok
  auto it     = _requests.find(TaskIndex);

  if ((request.status == P089_request_status::Request) &&
      ((it == _requests.end()) ||
       ((it->first == TaskIndex) && (it->second.status == P089_request_status::Finished)))) { // No request or Finished for this task
    _requests[TaskIndex]             = request;
    _requests[TaskIndex].status      = P089_request_status::Accepted;
    _requests[TaskIndex].lastStarted = millis();
    result                           = true;
  }

  return result;
}

bool P089_ping_service_struct::getPingResult(taskIndex_t        TaskIndex,
                                             P089_ping_request& request) {
  bool result = false;
  auto it     = _requests.find(TaskIndex);

  if (it != _requests.end()) {
    request = it->second;

    if (it->second.status == P089_request_status::Result) {
      it->second.status = P089_request_status::Finished; // Can be removed
      result            = true;
    }
  }

  // Check if we need to clean outdated requests
  it = _requests.begin();

  while (it != _requests.end()) {
    // Cleanup when counter reached
    if (timePassedSince(it->second.lastStarted) >= P089_FINISHED_REQUEST_TIMEOUT_MS) {
      addLog(LOG_LEVEL_INFO, strformat(F("PING : Clean last request for task %d (%u s)"),
                                       it->first + 1, timePassedSince(it->second.lastStarted) / 1000));
      _requests.erase(it);    // Cleanup
      it = _requests.begin(); // Restart loop
    } else {
      ++it;
    }
  }
  return result;
}

bool P089_ping_service_struct::loop() {
  bool result = false;

  if (timePassedSince(lastLooped) < 100) { // Some call/load throtteling
    return result;
  }
  lastLooped = millis();

  // addLog(LOG_LEVEL_INFO, F("PING : Running P089_ping_service->loop()")); // FIXME

  if (_ping_task_data.status == P089_task_status::Ready) {
    auto it = _requests.find(_ping_task_data.taskIndex);

    if (it != _requests.end()) {
      it->second.result  = _ping_task_data.result;
      it->second.avgTime = _ping_task_data.avgTime;
      it->second.status  = P089_request_status::Result;      // Can be retrieved
      result             = true;
    }
    _ping_task_data.status = P089_task_status::Initial;      // Available for new job
  }

  if (_ping_task_data.status == P089_task_status::Initial) { // Find next job
    auto it = _requests.begin();

    for (; it != _requests.end(); ++it) {
      if (it->second.status == P089_request_status::Accepted) {
        _ping_task_data.taskIndex = it->first;
        _ping_task_data.status    = P089_task_status::Working;
        _ping_task_data.count     = it->second.count;
        _ping_task_data.ip        = it->second.ip;
        _ping_task_data.espPing   = espPing;

        xTaskCreatePinnedToCore(
          P089_execute_ping_task,      // Function that should be called
          "PingClass.ping()",          // Name of the task (for debugging)
          4000,                        // Stack size (bytes)
          &_ping_task_data,            // Parameter to pass
          1,                           // Task priority
          &_ping_task_data.taskHandle, // Task handle
          xPortGetCoreID()             // Core you want to run the task on (0 or 1)
          );
        break;
      }
    }
  }
  return result;
}

#endif // ifdef ESP32
