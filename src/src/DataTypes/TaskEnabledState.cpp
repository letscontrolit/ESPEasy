#include "../DataTypes/TaskEnabledState.h"

TaskEnabledState::TaskEnabledState() : value(0) {}

TaskEnabledState & TaskEnabledState::operator=(const bool& enabledState)
{
  enabled = enabledState;
  return *this;
}

void TaskEnabledState::clearTempDisableFlags()
{
  const bool enabledSet = enabled;

  value   = 0;
  enabled = enabledSet;
}

void TaskEnabledState::setRetryInit()
{
  // If intended state is enabled, set to retry.
  retryInit = enabled;
}
