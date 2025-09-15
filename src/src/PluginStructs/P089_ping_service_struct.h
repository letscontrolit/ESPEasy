#pragma once

#include "../../_Plugin_Helper.h"

#ifdef ESP32

# include <ESPping.h>
# include <map>

constexpr uint32_t P089_FINISHED_REQUEST_TIMEOUT_MS = 1000 * 60 * 10; // Delete entry after unused for this number of milliseconds (10 min.)

struct P089_ping_service_struct;                                      // Forward declaration

extern P089_ping_service_struct*P089_ping_service;

enum class P089_task_status : uint8_t {
  Initial = 0, // Ready to be start a task
  Working = 1, // Working on a task
  Ready   = 2, // Ready, data is available
};

enum class P089_request_status : uint8_t {
  Request    = 0, // Initial request
  Accepted   = 1, // Can be processed
  Processing = 2, // Actively being processed
  Ready      = 3, // Results in
  Result     = 4, // Results retrieved
  Finished   = 5, // Can be removed or reused
};

struct P089_ping_task_data {
  taskIndex_t      taskIndex{};
  P089_task_status status = P089_task_status::Initial;
  IPAddress        ip;
  bool             result{};
  uint16_t         count{};
  float            avgTime{};
  PingClass       *espPing = nullptr;

  // This is C-code, so not set to nullptr, but to NULL
  TaskHandle_t taskHandle = NULL;
};

struct P089_ping_request {
  P089_request_status status = P089_request_status::Request;
  IPAddress           ip;
  String              hostname;
  uint16_t            count{};
  bool                result{};
  float               avgTime{};
  uint32_t            lastStarted{};
};

struct P089_ping_service_struct {
public:

  P089_ping_service_struct();
  virtual ~P089_ping_service_struct();

  bool isInitialized() {
    return nullptr != espPing;
  }

  bool    addPingRequest(taskIndex_t       TaskIndex,
                         P089_ping_request request);
  bool    getPingResult(taskIndex_t        TaskIndex,
                        P089_ping_request& request);
  bool    loop();

  uint8_t increment() {
    return ++taskCounter; // No checks needed?
  }

  uint8_t decrement() {
    return --taskCounter; // No checks needed?
  }

private:

  uint32_t lastLooped{};
  uint8_t  taskCounter{};

  std::map<taskIndex_t, P089_ping_request>_requests;
  PingClass                              *espPing = nullptr;
  P089_ping_task_data                     _ping_task_data;
};

#endif // ifdef ESP32
