#include "../PluginStructs/P001_data_struct.h"

#ifdef USES_P001


# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"

// TD-er: Needed to fix a mistake in earlier fixes.
uint8_t P001_data_struct::P001_getSwitchType(struct EventStruct *event)
{
  const uint8_t choice = P001_SWITCH_OR_DIMMER;

  if ((choice == 2) || // Old implementation for Dimmer
      (choice == PLUGIN_001_TYPE_DIMMER))
  {
    return PLUGIN_001_TYPE_DIMMER;
  }
  return PLUGIN_001_TYPE_SWITCH;
}

P001_data_struct::P001_data_struct(struct EventStruct *event) :
  _data(
    PLUGIN_GPIO,
    CONFIG_PIN1,
    P001_DEBOUNCE,
    P001_DC_MAX_INT,
    P001_LP_MIN_INT,
    P001_DOUBLECLICK,
    P001_LONGPRESS,
    P001_SAFE_BTN != 0,
    P001_BOOTSTATE,
    (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) ? SWITCH_TYPE_DIMMER : P001_BUTTON_TYPE,
    P001_DIMMER_VALUE)
{
  uint8_t pinModeValue = PIN_MODE_INPUT;

  // setPinState(PLUGIN_ID_001, _data._pin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  //  if it is in the device list we assume it's an input pin
  if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
  {
    setInternalGPIOPullupMode(_data._pin);
    pinModeValue = PIN_MODE_INPUT_PULLUP;
  }
  else
  {
    pinMode(_data._pin, INPUT);
    pinModeValue = PIN_MODE_INPUT;
  }


  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  const int8_t pinState = GPIO_Read_Switch_State(event);

  _data.init(
    event,
    pinState,
    pinModeValue);
}

void P001_data_struct::tenPerSecond(struct EventStruct *event)
{
  if (_data._pin == -1) { return; }

  portStatusStruct currentStatus = globalMapPortStatus[_data._portStatus_key];

  const int8_t pinState = GPIO_Read_Switch_State(_data._pin, currentStatus.mode);

  _data.tenPerSecond(event, F("GPIO"), pinState);
}

#endif // ifdef USES_P001
