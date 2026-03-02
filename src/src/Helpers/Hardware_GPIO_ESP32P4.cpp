#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP32P4
# include <driver/gpio.h>

# include "../Globals/Settings.h"
# include "../Helpers/Hardware_device_info.h"

#include <pins_arduino.h>

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************

bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

  input   = GPIO_IS_VALID_GPIO(gpio);
  output  = GPIO_IS_VALID_OUTPUT_GPIO(gpio);
  warning = isBootStrapPin(gpio);

  if ((gpio < 0) || !(GPIO_IS_VALID_GPIO(gpio))) { return false; }

  // P4 does not use GPIO-pins for accessing flash or PSRAM

# if FEATURE_ETHERNET

  // Check pins used for RMII Ethernet PHY
  if (Settings.isEthernetPin(gpio) || Settings.isEthernetPinOptional(gpio))
  {
    warning = true;
  }
# endif // if FEATURE_ETHERNET

#ifdef BOARD_HAS_SDIO_ESP_HOSTED
    // TODO TD-er: Make this configurable as we can set this via WiFi.setPins
    switch (gpio) {
      case BOARD_SDIO_ESP_HOSTED_CLK   : 
      case BOARD_SDIO_ESP_HOSTED_CMD   : 
      case BOARD_SDIO_ESP_HOSTED_D0    : 
      case BOARD_SDIO_ESP_HOSTED_D1    : 
      case BOARD_SDIO_ESP_HOSTED_D2    : 
      case BOARD_SDIO_ESP_HOSTED_D3    : 
      case BOARD_SDIO_ESP_HOSTED_RESET : 
      {
        warning = true;
//        if (gpio != BOARD_SDIO_ESP_HOSTED_RESET) { return false; }
        break;
      }
    }
#endif

  return (input || output);
}

bool isBootModePin(int gpio)
{
  return gpio == 35; 
}


bool isBootStrapPin(int gpio)
{
  //  Boot mode           | GPIO35 | GPIO36 | GPIO37 | GPIO38
  //  
  //  SPI Boot            |    1   |   Any  |   Any  |   Any
  //  Joint Download Boot |    0   |    1   |   Any  |   Any
  return gpio >= 35 && gpio <= 38;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  int  pinnr;
  bool input;
  bool output;
  bool warning;

  if (!getGpioInfo(gpio, pinnr, input, output, warning)) {
    return false;
  }

  hasPullUp   = true;
  hasPullDown = true;

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

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int gpio_pin, int& adc, int& ch, int& t)
{
  adc = -1;
  ch  = -1;
  t   = -1;

  switch (gpio_pin) {
    case A0:  adc = 1; ch = 0; break;
    case A1:  adc = 1; ch = 1; break;
    case A2:  adc = 1; ch = 2; break;
    case A3:  adc = 1; ch = 3; break;
    case A4:  adc = 1; ch = 4; break;
    case A5:  adc = 1; ch = 5; break;
    case A6:  adc = 1; ch = 6; break;
    case A7:  adc = 1; ch = 7; break;
    case A8:  adc = 2; ch = 0; break;
    case A9:  adc = 2; ch = 1; break;
    case A10: adc = 2; ch = 2; break;
    case A11: adc = 2; ch = 3; break;
    case A12: adc = 2; ch = 4; break;
    case A13: adc = 2; ch = 5; break;
    default:
      return false;
  }

  return true;
}

#if SOC_TOUCH_SENSOR_SUPPORTED
int touchPinToGpio(int touch_pin)
{
  if (touch_pin >= 0 && touch_pin < SOC_TOUCH_SENSOR_NUM) {
    return touch_pin + 2;
  }
  return -1;
}
#endif

#if SOC_DAC_SUPPORTED
// Get DAC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param dac        Number of DAC unit
bool getDAC_gpio_info(int gpio_pin, int& dac)
{
  return false;
}
#endif

#endif
