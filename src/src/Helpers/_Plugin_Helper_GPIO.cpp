#include "../Helpers/_Plugin_Helper_GPIO.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Helpers/PortStatus.h"


GPIO_plugin_helper_data_t::GPIO_plugin_helper_data_t(
  pluginID_t pluginNumber,
  uint16_t   pin,
  uint32_t   debounceInterval_ms,
  uint32_t   doubleClickMaxInterval_ms,
  uint32_t   longpressMinInterval_ms,
  uint8_t    dcMode,
  bool       safeButton,
    bool       sendBootState) :
  _debounceInterval_ms(debounceInterval_ms),
  _doubleClickMaxInterval_ms(doubleClickMaxInterval_ms),
  _longpressMinInterval_ms(longpressMinInterval_ms),
  _doubleClickCounter(0),
  _safeButtonCounter(0),
  _pin(pin),
  _dcMode(dcMode),
  _safeButton(safeButton),
  _longpressFired(false),
  _sendBootState(sendBootState)
{
  _portStatus_key = createKey(pluginNumber, _pin);

  // store millis for debounce, doubleclick and long press
  const unsigned long cur_millis = millis();

  _debounceTimer    = cur_millis; // debounce timer
  _doubleClickTimer = cur_millis; // doubleclick timer
  _longpressTimer   = cur_millis; // longpress timer
}

GPIO_plugin_helper_data_t::~GPIO_plugin_helper_data_t()
{
  if (_portStatus_key != 0) {
    removeTaskFromPort(_portStatus_key);
  }
}


bool GPIO_plugin_helper_data_t::init(
  struct EventStruct *event,
  int8_t              pinState,
  uint8_t             pinModeValue)
{
  // Read current status or create empty if it does not exist
  portStatusStruct newStatus = globalMapPortStatus[_portStatus_key];

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
  if (_sendBootState) {
    newStatus.state = !newStatus.state;

    newStatus.output = !newStatus.output; // FIXME TD-er: This was only done in P001, should this be done for PCF/MCP too?
  }

  // setPinState(PLUGIN_ID_009, _mcpPin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  savePortStatus(_portStatus_key, newStatus);
  return true;
}
