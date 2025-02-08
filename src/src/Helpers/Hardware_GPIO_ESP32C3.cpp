#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP32C3
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


  // FIXME TD-er: Implement for ESP32-C3
  // See:
  // - https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html
  // - https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/gpio.html
  // Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf

  if (gpio == 11) {
    if (getChipFeatures().embeddedFlash /* || !flashVddPinCanBeUsedAsGPIO() */) {
      // See: https://www.letscontrolit.com/forum/viewtopic.php?p=71880#p71874
      //
      // By default VDD_SPI is the power supply pin for embedded flash or external flash.
      // It can only be used as GPIO11 only when the chip is connected to an
      // external flash, and this flash is powered by an external power supply
      input  = false;
      output = false;
    }
    warning = true;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    if (getChipFeatures().embeddedFlash) {
      // Connected to the integrated SPI flash.
      input  = false;
      output = false;
    } else {
      // See: https://www.letscontrolit.com/forum/viewtopic.php?p=71880#p71874
      if ((gpio == 12) || (gpio == 13)) {
        // SPIHD/GPIO12
        // SPIWP/GPIO13
        if ((ESP.getFlashChipMode() != FM_DOUT) &&
            (ESP.getFlashChipMode() != FM_DIO)) {
          input  = false;
          output = false;
        }
      }
    }
    warning = true;
  }

  if ((gpio == PIN_USB_D_MIN) || (gpio == PIN_USB_D_PLUS)) {
    // USB OTG and USB Serial/JTAG function. USB signal is a differential
    // signal transmitted over a pair of D+ and D- wires.
    warning = true;
  }

  // GPIO 18: USB_D-
  // GPIO 19: USB_D+

  // GPIO 20: U0RXD
  // GPIO 21: U0TXD

  return (input || output);
}

bool isBootStrapPin(int gpio)
{
  if (gpio == 2) {
    // Strapping pin which must be high during boot
    return true;
  }

  if (gpio == 8) {
    // Strapping pin which must be high during flashing
    return true;
  }

  if (gpio == 9) {
    // Strapping pin to force download mode (like GPIO-0 on ESP8266/ESP32-classic)
    return true;
  }
  return false;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  if (validGpio(gpio)) {
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

  if ((gpio_pin >= 0) && (gpio_pin <= 4)) {
    adc = 1;
    ch  = gpio_pin;
    return true;
  }
  # if ESP_IDF_VERSION_MAJOR >= 5

  // Support for ADC2 has been dropped.
  # else // if ESP_IDF_VERSION_MAJOR >= 5

  if (gpio_pin == 5) {
    adc = 2;
    ch  = 0;
    return true;
  }
  # endif // if ESP_IDF_VERSION_MAJOR >= 5
  return false;
}

int touchPinToGpio(int touch_pin)
{
  // No touch pin support
  return -1;
}

// Get DAC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param dac        Number of DAC unit
bool getDAC_gpio_info(int gpio_pin, int& dac)
{
  // ESP32-C3, ESP32-S3, ESP32-C2, ESP32-C6 and ESP32-H2 don't have a DAC onboard
  return false;
}

#endif // ifdef ESP32C3
