#ifndef DATATYPES_TASKENABLEDSTATE_H
#define DATATYPES_TASKENABLEDSTATE_H

#include "../../ESPEasy_common.h"

// Is stored in settings
struct TaskEnabledState {

  TaskEnabledState();

//  bool isEnabled() const;
  explicit operator bool() const {
    return value == 1;
  }

  TaskEnabledState& operator=(const bool& enabledState);

  void clearTempDisableFlags();

  void setRetryInit();

  union {
    struct {
      uint8_t enabled   : 1;
      uint8_t retryInit : 1;
      uint8_t unused    : 6;
    };

    uint8_t value{};
  };
};


#endif // ifndef DATATYPES_TASKENABLEDSTATE_H
