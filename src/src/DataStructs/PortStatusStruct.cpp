#include "PortStatusStruct.h"

#include "../DataStructs/PinMode.h"

portStatusStruct::portStatusStruct() : state(-1), output(-1), command(0), init(0), not_used(0), mode(0), task(0), monitor(0), forceMonitor(0),
  forceEvent(0), previousTask(-1), x(INVALID_DEVICE_INDEX) {}

uint16_t portStatusStruct::getDutyCycle() const
{
  if (mode == PIN_MODE_PWM) {
    return dutyCycle;
  }
  return 0;
}

int16_t portStatusStruct::getValue() const
{
  switch (mode) {
    case PIN_MODE_PWM:
    case PIN_MODE_SERVO:
      return dutyCycle;
    default:
      break;
  }
  return state;
}