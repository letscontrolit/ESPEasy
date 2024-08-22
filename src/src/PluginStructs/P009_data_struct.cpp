#include "../PluginStructs/P009_data_struct.h"

#ifdef USES_P009

# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"


uint8_t P009_data_struct::getI2C_address(struct EventStruct *event)
{
  const uint8_t unit = (CONFIG_PORT - 1) / 16;

  return 0x20 + unit;
}

P009_data_struct::P009_data_struct(struct EventStruct *event) :
  _data(
    PLUGIN_MCP,
    CONFIG_PORT,
    P009_DEBOUNCE,
    P009_DC_MAX_INT,
    P009_LP_MIN_INT,
    P009_DOUBLECLICK,
    P009_LONGPRESS,
    P009_SAFE_BTN != 0,
    P009_BOOTSTATE)
# if FEATURE_I2C_DEVICE_CHECK
  , _address(P009_data_struct::getI2C_address(event))
# endif // if FEATURE_I2C_DEVICE_CHECK
{
  // Turn on Pullup resistor
  setMCPInputAndPullupMode(_data._pin, true);

  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  const int8_t state = GPIO_MCP_Read(_data._pin);

  _data.init(
    event,
    state,
    (state == -1) ?  PIN_MODE_OFFLINE : PIN_MODE_INPUT_PULLUP);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           concat(F("MCP INIT="), state));
  }
}

void P009_data_struct::tenPerSecond(struct EventStruct *event)
{
# if FEATURE_I2C_DEVICE_CHECK

  if (!I2C_deviceCheck(_address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) { // Generate stats
    return;                                                                       // Will return the default false for success
  }
# endif // if FEATURE_I2C_DEVICE_CHECK

  const int8_t state = GPIO_MCP_Read(_data._pin);

  _data.tenPerSecond(event, F("MCP"), state);
}

#endif // ifdef USES_P009
