#include "../Helpers/Hardware_GPIO.h"

#ifdef ESP32S3
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


  // FIXME TD-er: Implement for ESP32-S3
  // See:
  // - https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html
  // Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf

  if ((gpio >= 26) && (gpio <= 37)) {
    // Connected to the integrated SPI flash.
    // SPI0/1: GPIO26-32 are usually used for SPI flash and PSRAM and not recommended for other uses.
    // When using Octal Flash or Octal PSRAM or both,
    //  GPIO33~37 are connected to SPIIO4 ~ SPIIO7 and SPIDQS.
    // Therefore, on boards embedded with ESP32-S3R8 / ESP32-S3R8V chip,
    //  GPIO33~37 are also not recommended for other uses.
    input   = gpio > 32;
    output  = gpio > 32;
    warning = true;
  }


  if ((gpio == 19) || (gpio == 20)) {
    // USB OTG and USB Serial/JTAG function. USB signal is a differential
    // signal transmitted over a pair of D+ and D- wires.
    warning = true;
  }

  return (input || output);
}

bool isBootStrapPin(int gpio)
{
  if (gpio == 45) {
    // GPIO45 is used to select the VDD_SPI power supply voltage at reset:
    // • GPIO45 = 0, VDD_SPI pin is powered directly from VDD3P3_RTC via resistor RSP I . Typically this voltage is
    //   3.3 V. For more information, see Figure: ESP32-S3 Power Scheme in ESP32-S3 Datasheet.
    // • GPIO45 = 1, VDD_SPI pin is powered from internal 1.8 V LDO.
    return true;
  }

  if (gpio == 46) {
    // Strapping pin which must be low during flashing
    return true;
  }

  // GPIO 0  State during boot determines boot mode.
  if (gpio == 0) { return true; }

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
  // ESP32-C3, ESP32-S3, ESP32-C2, ESP32-C6 and ESP32-H2 don't have a DAC onboard
  return false;
}

#endif // ifdef ESP32S3
