#include "../Helpers/Scheduler.h"

#include "../DataStructs/PinMode.h"
#include "../DataStructs/Scheduler_GPIOTimerID.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"

#include "../Globals/GlobalMapPortStatus.h"

#include "../Helpers/PortStatus.h"

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
void ESPEasy_Scheduler::setGPIOTimer(
  unsigned long msecFromNow,
  pluginID_t    pluginID,
  int           pinnr,
  int           state,
  int           repeatInterval,
  int           recurringCount,
  int           alternateInterval)
{
  uint8_t GPIOType = GPIO_TYPE_INVALID;

  switch (pluginID.value) {
    case PLUGIN_GPIO_INT:
      GPIOType = GPIO_TYPE_INTERNAL;
      break;
    case PLUGIN_PCF_INT:
      GPIOType = GPIO_TYPE_PCF;
      break;
    case PLUGIN_MCP_INT:
      GPIOType = GPIO_TYPE_MCP;
      break;
  }

  if (GPIOType != GPIO_TYPE_INVALID) {
    // Par1 & Par2 & GPIOType form a unique key
    const GPIOTimerID timerID(GPIOType, pinnr, state);
    const systemTimerStruct timer_data(
      recurringCount,
      repeatInterval,
      state,
      alternateInterval);
    systemTimers[timerID.mixed_id] = timer_data;
    setNewTimerAt(timerID, millis() + msecFromNow);
  }
}

void ESPEasy_Scheduler::clearGPIOTimer(pluginID_t pluginID, int pinnr)
{
  uint8_t GPIOType = GPIO_TYPE_INVALID;

  switch (pluginID.value) {
    case PLUGIN_GPIO_INT:
      GPIOType = GPIO_TYPE_INTERNAL;
      break;
    case PLUGIN_PCF_INT:
      GPIOType = GPIO_TYPE_PCF;
      break;
    case PLUGIN_MCP_INT:
      GPIOType = GPIO_TYPE_MCP;
      break;
  }

  if (GPIOType != GPIO_TYPE_INVALID) {
    // Par1 & Par2 & GPIOType form a unique key
    for (int state = 0; state <= 1; ++state) {
      const GPIOTimerID timerID(GPIOType, pinnr, state);
      auto it = systemTimers.find(timerID.mixed_id);

      if (it != systemTimers.end()) {
        systemTimers.erase(it);
      }
      msecTimerHandler.remove(timerID.mixed_id);
    }
  }
}

void ESPEasy_Scheduler::process_gpio_timer(SchedulerTimerID timerID, unsigned long lasttimer) {
  auto it = systemTimers.find(timerID.mixed_id);

  if (it == systemTimers.end()) {
    return;
  }

  // Reschedule before sending the event, as it may get rescheduled in handling the timer event.
  if (it->second.isRecurring()) {
    // Recurring timer
    it->second.markNextRecurring();

    unsigned long newTimer = lasttimer;
    setNextTimeInterval(newTimer, it->second.getInterval());
    setNewTimerAt(timerID, newTimer);
  }

  const GPIOTimerID *tmp = reinterpret_cast<const GPIOTimerID *>(&timerID);

  uint8_t GPIOType      = tmp->getGPIO_type();
  uint8_t pinNumber     = tmp->getPinNumber();
  uint8_t pinStateValue = tmp->getPinStateValue();

  if (it->second.isAlternateState()) {
    pinStateValue = (pinStateValue > 0) ? 0 : 1;
  }

  pluginID_t pluginID = PLUGIN_GPIO;

  switch (GPIOType)
  {
    case GPIO_TYPE_INTERNAL:
      GPIO_Internal_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_GPIO;
      break;
#ifdef USES_P009
    case GPIO_TYPE_MCP:
      GPIO_MCP_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_MCP;
      break;
#endif // ifdef USES_P009
#ifdef USES_P019
    case GPIO_TYPE_PCF:
      GPIO_PCF_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_PCF;
      break;
#endif // ifdef USES_P019
    default:
      return;
  }


  if (!it->second.isRecurring()) {
    clearGPIOTimer(pluginID, pinNumber);
  }

  const uint32_t key = createKey(pluginID, pinNumber);

  // WARNING: operator [] creates an entry in the map if key does not exist
  portStatusStruct tempStatus = globalMapPortStatus[key];

  tempStatus.mode    = PIN_MODE_OUTPUT;
  tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page

  if (tempStatus.state != pinStateValue) {
    tempStatus.state        = pinStateValue;
    tempStatus.output       = pinStateValue;
    tempStatus.forceEvent   = 1;
    tempStatus.forceMonitor = 1;
  }
  savePortStatus(key, tempStatus);
}
