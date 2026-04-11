#include "../Helpers/Hardware_GPIO.h"
#include "ESPEasy_config.h"

#ifdef ESP8266

# include "../Helpers/Hardware_device_info.h"

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************

// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr  = -1;
  input  = true;
  output = true;

  warning = isBootStrapPin(gpio);

  constexpr int8_t NodeMCU_PINNR[]
  { 3, 10, 4, 9, 2, 1, 
    -1, -1, -1,  // GPIO 6 .. 8 & 11  is used for flash 
    11, 12,         // On ESP8266 GPIO 9 & 10 used for flash (QUAD mode)
    -1,                 // GPIO 11
    6, 7, 5, 8, 
    0                   // GPIO 16 is used by the deep-sleep mechanism
  };

  if ((gpio >= 0) && (static_cast<size_t>(gpio) < NR_ELEMENTS(NodeMCU_PINNR))) {
    pinnr = NodeMCU_PINNR[gpio];
  }

  if (gpio == 15) {
    // GPIO-15 Can't be used as an input. There is an external pull-down on this pin.
    input = false;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    if (isESP8285()) {
      if ((gpio == 9) || (gpio == 10)) {
        // Usable on ESP8285
      } else {
        warning = true;
      }
    } else {
      warning = true;

      // On ESP8266 GPIO 9 & 10 are only usable if not connected to flash
      if (gpio == 9) {
        // GPIO9 is internally used to control the flash memory.
        input  = false;
        output = false;
      } else if (gpio == 10) {
        // GPIO10 can be used as input only.
        output = false;
      }
    }
  }

  if ((pinnr < 0) || (pinnr > 16)) {
    input  = false;
    output = false;
  }
  return input || output;
}

bool isBootModePin(int gpio)  { return gpio == 0; }

bool isBootStrapPin(int gpio) { return gpio == 0 || gpio == 2 || gpio == 15; }

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  if (!validGpio(gpio)) {
    return false;
  }

  if (gpio == 16) {
    hasPullDown = true;
  } else {
    hasPullUp = true;
  }
  return true;
}

bool validGpio(int gpio) {
  if (gpio < 0) { return false; }

  if (!GPIO_IS_VALID_GPIO(gpio)) { return false; }

  int  pinnr;
  bool input;
  bool output;
  bool warning;

  return getGpioInfo(gpio, pinnr, input, output, warning);
}

#endif // ifdef ESP8266
