#ifndef DATATYPES_INTENDEDREBOOTREASON_H
#define DATATYPES_INTENDEDREBOOTREASON_H

#include "../../ESPEasy_common.h"

enum class IntendedRebootReason_e : uint8_t {
  DeepSleep,
  DelayedReboot,
  ResetFactory,
  ResetFactoryPinActive,
  ResetFactoryCommand,
  CommandReboot,
  RestoreSettings,
  OTA_error,
  ConnectionFailuresThreshold,
};

String toString(IntendedRebootReason_e reason);


#endif // ifndef DATATYPES_INTENDEDREBOOTREASON_H
