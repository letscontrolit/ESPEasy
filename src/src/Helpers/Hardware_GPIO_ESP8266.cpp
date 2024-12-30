#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP8266

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************


// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr  = -1;
  input  = true;
  output = true;

  warning = isBootStrapPin(gpio);

  switch (gpio) {
    case  0: pinnr =  3; break;
    case  1: pinnr = 10; break;
    case  2: pinnr =  4; break;
    case  3: pinnr =  9; break;
    case  4: pinnr =  2; break;
    case  5: pinnr =  1; break;
    case  6:                    // GPIO 6 .. 8  is used for flash
    case  7:
    case  8: pinnr = -1; break;
    case  9: pinnr = 11; break; // On ESP8266 used for flash
    case 10: pinnr = 12; break; // On ESP8266 used for flash
    case 11: pinnr = -1; break;
    case 12: pinnr =  6; break;
    case 13: pinnr =  7; break;
    case 14: pinnr =  5; break;

    // GPIO-15 Can't be used as an input. There is an external pull-down on this pin.
    case 15: pinnr =  8; input = false; break;
    case 16: pinnr =  0; break; // This is used by the deep-sleep mechanism
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

bool isBootStrapPin(int gpio)
{
    return (gpio == 0 || gpio == 2 || gpio == 15);
}

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
  if (gpio > MAX_GPIO) { return false; }

  int pinnr;
  bool input;
  bool output;
  bool warning;

  return getGpioInfo(gpio, pinnr, input, output, warning);
}


#endif // ifdef ESP8266