#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP32C5
# include <driver/gpio.h>
# include <soc/spi_pins.h>

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


  // FIXME TD-er: Implement for ESP32-C5
  // See:
  // - https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c5/hw-reference/esp32c3/user-guide-devkitm-1.html
  // - https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-reference/peripherals/gpio.html
  // Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf

  if (isFlashInterfacePin_ESPEasy(gpio) || isPSRAMInterfacePin(gpio)) {
    if (getChipFeatures().embeddedFlash) {
      // Connected to the integrated SPI flash.
      input  = false;
      output = false;
    } else {
      // See: https://www.letscontrolit.com/forum/viewtopic.php?p=71880#p71874
      if ((gpio == MSPI_IOMUX_PIN_NUM_HD) || (gpio == MSPI_IOMUX_PIN_NUM_WP)) {
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

  return (input || output);
}

bool isBootModePin(int gpio)
{
  return gpio == 28; 
}

bool isBootStrapPin(int gpio)
{
  //  Boot mode             | GPIO26 | GPIO27 | GPIO28
  //  
  //  SPI Boot              |   Any  |   Any  |    1
  //  Joint Download Boot 0 |   Any  |    1   |    0
  //  Joint Download Boot 1 |    0   |    0   |    0   
/*
  Joint Download Boot 0 mode supports the following download methods:
  - USB-Serial-JTAG Download Boot
  - UART Download Boot
  - SPI Slave Download Boot (chip revision v0.1 only)

  Joint Download Boot 1 mode supports the following download methods:
  - UART Download Boot
  - SDIO Download Boot
*/
  if (gpio == 26 || gpio == 27 || gpio == 28) {
    // Strapping pin setting boot mode
    return true;
  }


  if (gpio == 7) {
    // Strapping pin JTAG signal source
    return true;
  }

  // Ignoring strapping pins GPIO-2, -3, 25
  // as they should never cause issues for users.
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

  // GPIO 1 ... 6 -> Channel 0 ... 5
  if ((gpio_pin > 0) && (gpio_pin <= 6)) {
    adc = 1;
    ch  = gpio_pin - 1;
    return true;
  }
  return false;
}

#if SOC_TOUCH_SENSOR_SUPPORTED
int touchPinToGpio(int touch_pin)
{
  // No touch pin support
  return -1;
}
#endif

#if SOC_DAC_SUPPORTED
// Get DAC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param dac        Number of DAC unit
bool getDAC_gpio_info(int gpio_pin, int& dac)
{
  // ESP32-C5, ESP32-S3, ESP32-C2, ESP32-C6 and ESP32-H2 don't have a DAC onboard
  return false;
}
#endif

#endif // ifdef ESP32C5
