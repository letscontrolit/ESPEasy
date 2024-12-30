#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP32S2
# include <driver/gpio.h>

# include "../Helpers/Hardware_device_info.h"

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************

bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

  input   = GPIO_IS_VALID_GPIO(gpio);
  output  = GPIO_IS_VALID_OUTPUT_GPIO(gpio);
  warning = isBootStrapPin(gpio);

  if (!(GPIO_IS_VALID_GPIO(gpio))) { return false; }

  // Input GPIOs:  0-21, 26, 33-46
  // Output GPIOs: 0-21, 26, 33-45
  input  = gpio <= 46;
  output = gpio <= 45;

  if ((gpio < 0) || ((gpio > 21) && (gpio < 33) && (gpio != 26))) {
    input  = false;
    output = false;
  }

  if (FoundPSRAM() && (gpio == 26)) {
    // Pin shared with the flash memory and/or PSRAM.
    // Cannot be used as regular GPIO
    input   = false;
    output  = false;
    warning = true;
  }

  if ((gpio > 26) && (gpio < 33)) {
    // SPIHD, SPIWP, SPICS0, SPICLK, SPIQ, SPID pins of ESP32-S2FH2 and ESP32-S2FH4
    // are connected to embedded flash and not recommended for other uses.
    warning = true;
  }

  return (input || output);
}

bool isBootStrapPin(int gpio)
{
  if (gpio == 45) {
    // VDD_SPI can work as the power supply for the external device at either
    // 1.8 V (when GPIO45 is 1 during boot), or
    // 3.3 V (when GPIO45 is 0 and at default state during boot).
    return true;
  }

  // GPIO 0  State during boot determines boot mode.
  if (gpio == 0) { return true; }

  if (gpio == 46) {
    // Strapping pin which must be low during flashing
    return true;
  }

  return false;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  if (!validGpio(gpio)) {
    return false;
  }

  // GPI: GPIO46 is fixed to pull-down and is input only.
  if (gpio <= 45) {
    hasPullUp   = true;
    hasPullDown = true;
  }
  return true;
}

bool validGpio(int gpio) {
  if (!GPIO_IS_VALID_GPIO(gpio)) { return false; }

  int  pinnr;
  bool input;
  bool output;
  bool warning;

  return getGpioInfo(gpio, pinnr, input, output, warning);
}

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit (0 == Hall effect)
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int gpio_pin, int& adc, int& ch, int& t)
{
  adc = -1;
  ch  = -1;
  t   = -1;

  switch (gpio_pin) {
    case 1: adc  = 1; ch = 0; t = 1; break;
    case 2: adc  = 1; ch = 1; t = 2; break;
    case 3: adc  = 1; ch = 2; t = 3; break;
    case 4: adc  = 1; ch = 3; t = 4; break;
    case 5: adc  = 1; ch = 4; t = 5; break;
    case 6: adc  = 1; ch = 5; t = 6; break;
    case 7: adc  = 1; ch = 6; t = 7; break;
    case 8: adc  = 1; ch = 7; t = 8; break;
    case 9: adc  = 1; ch = 8; t = 9; break;
    case 10: adc = 1; ch = 9; t = 10; break;
    case 11: adc = 2; ch = 0; t = 11; break;
    case 12: adc = 2; ch = 1; t = 12; break;
    case 13: adc = 2; ch = 2; t = 13; break;
    case 14: adc = 2; ch = 3; t = 14; break;
    case 15: adc = 2; ch = 4;  break;
    case 16: adc = 2; ch = 5;  break;
    case 17: adc = 2; ch = 6;  break;
    case 18: adc = 2; ch = 7;  break;
    case 19: adc = 2; ch = 8;  break;
    case 20: adc = 2; ch = 9;  break;
    default:
      return false;
  }

  return true;
}

int touchPinToGpio(int touch_pin)
{
  switch (touch_pin) {
    case 1: return T1;
    case 2: return T2;
    case 3: return T3;
    case 4: return T4;
    case 5: return T5;
    case 6: return T6;
    case 7: return T7;
    case 8: return T8;
    case 9: return T9;
    case 10: return T10;
    case 11: return T11;
    case 12: return T12;
    case 13: return T13;
    case 14: return T14;
    default:
      break;
  }

  return -1;
}

// Get DAC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param dac        Number of DAC unit
bool getDAC_gpio_info(int gpio_pin, int& dac)
{
  switch (gpio_pin) {
    case 17: dac = 1; break;
    case 18: dac = 2; break;
    default:
      return false;
  }
  return true;
}

#endif // ifdef ESP32S2
