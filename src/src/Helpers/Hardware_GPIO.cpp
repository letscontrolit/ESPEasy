#include "../Helpers/Hardware_GPIO.h"


#include "../Globals/Settings.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_device_info.h"
#include "../Helpers/StringConverter.h"

#ifdef ESP32
# include <esp32-hal-periman.h>
# include <driver/gpio.h>
# include <esp_private/esp_gpio_reserve.h>

// # include <hal/gpio_types.h>
 # include <hal/gpio_hal.h>

// #include <hal/gpio_ll.h>

# include <esp_private/io_mux.h>
#endif // ifdef ESP32


// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************


bool isSerialConsolePin(int gpio) {
  if (!Settings.UseSerial) { return false; }

#if defined(SOC_RX0) && defined(SOC_TX0)
  return (gpio == SOC_TX0 || gpio == SOC_RX0)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;
#else
  static_assert(false, "Implement processor architecture");
  return false;
#endif
}


#ifdef ESP32
bool getPeriman_gpio_info(int  gpio_pin,
  peripheral_bus_type_t& bus_type,
  String& gpio_str,
    String& typeName,int& bus_number, int& bus_channel)
{
    if (!perimanPinIsValid(gpio_pin)) {
      return false; // invalid pin
    }
    bus_type = perimanGetPinBusType(gpio_pin);

    if (bus_type == ESP32_BUS_TYPE_INIT) {
      return false; // unused pin
    }

#  if defined(BOARD_HAS_PIN_REMAP)
    int dpin = gpioNumberToDigitalPin(gpio_pin);

    if (dpin < 0) {
      return false; // pin is not exported
    } else {
      gpio_str = strformat(F("D%-3d|%4u"), dpin, gpio_pin);
    }
#  else // if defined(BOARD_HAS_PIN_REMAP)
    gpio_str = strformat(F("%4u"), gpio_pin);
#  endif // if defined(BOARD_HAS_PIN_REMAP)
    const char *extra_type = perimanGetPinBusExtraType(gpio_pin);

    if (extra_type) {
      typeName = strformat(F("%s [%s]"), perimanGetTypeName(bus_type), extra_type);
    } else {
      typeName = perimanGetTypeName(bus_type);
    }
    bus_number = perimanGetPinBusNum(gpio_pin);
    bus_channel = perimanGetPinBusChannel(gpio_pin);
    return true;
}


#endif