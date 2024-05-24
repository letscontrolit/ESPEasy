#include "../DataStructs/Scheduler_GPIOTimerID.h"

#include "../DataStructs/PinMode.h"

GPIOTimerID::GPIOTimerID(uint8_t GPIOType, uint8_t pinNumber, int Par1) :
  SchedulerTimerID(SchedulerTimerType_e::GPIO_timer)
{
  setId((Par1 << 16) + (pinNumber << 8) + GPIOType);
}


#ifndef BUILD_NO_DEBUG
String GPIOTimerID::decode() const
{
  String result;

  switch (getGPIO_type())
  {
    case GPIO_TYPE_INTERNAL:
      result += F("int");
      break;
    case GPIO_TYPE_MCP:
      result += F("MCP");
      break;
    case GPIO_TYPE_PCF:
      result += F("PCF");
      break;
    default:
      result += '?';
      break;
  }
  result += F(" pin: ");
  result += getPinNumber();
  result += F(" state: ");
  result += getPinStateValue();
  return result;
}

#endif // ifndef BUILD_NO_DEBUG
