#include "../PluginStructs/P019_data_struct.h"

#ifdef USES_P019

# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"

uint8_t P019_data_struct::getI2C_address(struct EventStruct *event)
{
  const uint8_t unit = (CONFIG_PORT - 1) / 8;

  //  const uint8_t port = CONFIG_PORT - (unit * 8);
  uint8_t address = 0x20 + unit;

  if (unit > 7) { address += 0x10; }

  return address;
}

P019_data_struct::P019_data_struct(struct EventStruct *event) :
  _data(
    PLUGIN_PCF,
    CONFIG_PORT,
    P019_DEBOUNCE,
    P019_DC_MAX_INT,
    P019_LP_MIN_INT,
    P019_DOUBLECLICK,
    P019_LONGPRESS,
    P019_SAFE_BTN != 0,
    P019_BOOTSTATE)
# if FEATURE_I2C_DEVICE_CHECK
  , _address(P019_data_struct::getI2C_address(event))
# endif // if FEATURE_I2C_DEVICE_CHECK
{
  // Turn on Pullup resistor
  setPCFInputMode(_data._pin);

  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  const int8_t state = GPIO_PCF_Read(_data._pin);

  _data.init(
    event,
    state,
    (state == -1) ?  PIN_MODE_OFFLINE : PIN_MODE_INPUT_PULLUP);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           concat(F("PCF INIT="), state));
  }
}

void P019_data_struct::tenPerSecond(struct EventStruct *event)
{
# if FEATURE_I2C_DEVICE_CHECK

  if (!I2C_deviceCheck(_address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) { // Generate stats
    return;                                                                       // Will return the default false for success
  }
# endif // if FEATURE_I2C_DEVICE_CHECK

  const int8_t state = GPIO_PCF_Read(_data._pin);

  _data.tenPerSecond(event, F("PCF"), state);
}

#endif // ifdef USES_P019
