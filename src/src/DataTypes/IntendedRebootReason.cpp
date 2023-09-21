#include "../DataTypes/IntendedRebootReason.h"

const __FlashStringHelper * toString_f(IntendedRebootReason_e reason) {
  switch (reason) {
    case IntendedRebootReason_e::DeepSleep:              return F("DeepSleep");
    case IntendedRebootReason_e::DelayedReboot:          return F("DelayedReboot");
    case IntendedRebootReason_e::ResetFactory:           return F("ResetFactory");
    case IntendedRebootReason_e::ResetFactoryPinActive:  return F("ResetFactoryPinActive");
    case IntendedRebootReason_e::ResetFactoryCommand:    return F("ResetFactoryCommand");
    case IntendedRebootReason_e::CommandReboot:          return F("CommandReboot");
    case IntendedRebootReason_e::RestoreSettings:        return F("RestoreSettings");
    case IntendedRebootReason_e::OTA_error:              return F("OTA_error");
    case IntendedRebootReason_e::ConnectionFailuresThreshold: return F("ConnectionFailuresThreshold");
  }
  return F("");
}

String toString(IntendedRebootReason_e reason) {
  String res = toString_f(reason);
  if (res.isEmpty()) return String(static_cast<int>(reason));
  return res;
}
