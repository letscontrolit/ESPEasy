#include "../Helpers/_Plugin_Helper_GPIO.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Helpers/PortStatus.h"

bool GPIO_plugin_helper_init(
  struct EventStruct *event,
  uint32_t            portStatus_key,
  int                 pin,
  int8_t              pinState,
  uint8_t             pinModeValue,
  bool                sendBootState)
{
  // Read current status or create empty if it does not exist
  portStatusStruct newStatus = globalMapPortStatus[portStatus_key];

  newStatus.state  = pinState;
  newStatus.output = newStatus.state; // FIXME TD-er: Is this correct to set output value based on state?
  newStatus.mode   = pinModeValue;

  // @giig1967g: if it is in the device list we assume it's an input pin
  // add this GPIO/port as a task
  if (newStatus.task < 3) {
    newStatus.task++;
  }


  // @giig1967g-20181022: set initial UserVar of the switch
  if ((newStatus.state != -1) && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
    UserVar.setFloat(event->TaskIndex, 0, !newStatus.state);
  } else {
    UserVar.setFloat(event->TaskIndex, 0, newStatus.state);
  }

  // if boot state must be send, inverse default state
  // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
  if (sendBootState) {
    newStatus.state = !newStatus.state;

    newStatus.output = !newStatus.output; // FIXME TD-er: This was only done in P001, should this be done for PCF/MCP too?
  }

  // setPinState(PLUGIN_ID_009, _mcpPin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  savePortStatus(portStatus_key, newStatus);
  return true;
}
