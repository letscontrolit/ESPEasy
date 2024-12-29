#include "../Helpers/Hardware_GPIO.h"


#include "../Globals/Settings.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_device_info.h"

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************

#ifdef ESP32
#include <driver/gpio.h>

bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

  input   = GPIO_IS_VALID_GPIO(gpio);
  output  = GPIO_IS_VALID_OUTPUT_GPIO(gpio);
  warning = false;

  if ((gpio < 0) || !(GPIO_IS_VALID_GPIO(gpio))) { return false; }

# ifdef ESP32S2

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


  if ((input == false) && (output == false)) {
    return false;
  }

  if (gpio == 45) {
    // VDD_SPI can work as the power supply for the external device at either
    // 1.8 V (when GPIO45 is 1 during boot), or
    // 3.3 V (when GPIO45 is 0 and at default state during boot).
    warning = true;
  }

  // GPIO 0  State during boot determines boot mode.
  if (gpio == 0) { warning = true; }

  if (gpio == 46) {
    // Strapping pin which must be low during flashing
    warning = true;
  }

# elif defined(ESP32S3)

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

  if (gpio == 45) {
    // GPIO45 is used to select the VDD_SPI power supply voltage at reset:
    // • GPIO45 = 0, VDD_SPI pin is powered directly from VDD3P3_RTC via resistor RSP I . Typically this voltage is
    //   3.3 V. For more information, see Figure: ESP32-S3 Power Scheme in ESP32-S3 Datasheet.
    // • GPIO45 = 1, VDD_SPI pin is powered from internal 1.8 V LDO.
    warning = true;
  }

  if (gpio == 46) {
    // Strapping pin which must be low during flashing
    warning = true;
  }

  if ((input == false) && (output == false)) {
    return false;
  }

  // GPIO 0  State during boot determines boot mode.
  if (gpio == 0) { warning = true; }

# elif defined(ESP32C2)

  if (gpio == 8) {
    // Strapping pin which must be high during flashing
    warning = true;
  }

  if (gpio == 9) {
    // Strapping pin to force download mode (like GPIO-0 on ESP8266/ESP32-classic)
    warning = true;
  }

  if (gpio == 11) {
    // By default VDD_SPI is the power supply pin for embedded flash or external flash. It can only be used as GPIO11
    // only when the chip is connected to an external flash, and this flash is powered by an external power supply
    input   = false;
    output  = false;
    warning = true;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    // Connected to the integrated SPI flash.
    input   = false;
    output  = false;
    warning = true;
  }

  if ((input == false) && (output == false)) {
    return false;
  }

# elif defined(ESP32C3)

// FIXME TD-er: Implement for ESP32-C3
// See: 
// - https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html
// - https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/gpio.html
// Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf

  if (gpio == 2) {
    // Strapping pin which must be high during boot
    warning = true;
  }

  if (gpio == 8) {
    // Strapping pin which must be high during flashing
    warning = true;
  }

  if (gpio == 9) {
    // Strapping pin to force download mode (like GPIO-0 on ESP8266/ESP32-classic)
    warning = true;
  }

  if (gpio == 11) {
    if (getChipFeatures().embeddedFlash) {
      // See: https://www.letscontrolit.com/forum/viewtopic.php?p=71880#p71874
      //
      // By default VDD_SPI is the power supply pin for embedded flash or external flash. 
      // It can only be used as GPIO11 only when the chip is connected to an 
      // external flash, and this flash is powered by an external power supply
      input   = false;
      output  = false;
    }
    warning = true;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    if (getChipFeatures().embeddedFlash) {
      // Connected to the integrated SPI flash.
      input   = false;
      output  = false;
    } else {
      // See: https://www.letscontrolit.com/forum/viewtopic.php?p=71880#p71874
      if (gpio == 12 || gpio == 13) {
        // SPIHD/GPIO12
        // SPIWP/GPIO13
        if (ESP.getFlashChipMode() != FM_DOUT && 
            ESP.getFlashChipMode() != FM_DIO) {
          input   = false;
          output  = false;
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


  if ((input == false) && (output == false)) {
    return false;
  }
  // GPIO 18: USB_D-
  // GPIO 19: USB_D+

  // GPIO 20: U0RXD
  // GPIO 21: U0TXD
  

  # elif defined(ESP32C6)

  if (gpio == 8) {
    // Strapping pin which must be high during flashing
    warning = true;
  }

  if (gpio == 9) {
    // Strapping pin to force download mode (like GPIO-0 on ESP8266/ESP32-classic)
    warning = true;
  }

  if (gpio == 27) {
    // By default VDD_SPI is the power supply pin for embedded flash or external flash. It can only be used as GPIO
    // only when the chip is connected to an external flash, and this flash is powered by an external power supply
    input   = false;
    output  = false;
    warning = true;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    // Connected to the integrated SPI flash.
    input   = false;
    output  = false;
    warning = true;
  }

  if ((gpio == PIN_USB_D_MIN) || (gpio == PIN_USB_D_PLUS)) {
    // USB OTG and USB Serial/JTAG function. USB signal is a differential
    // signal transmitted over a pair of D+ and D- wires.
    warning = true;
  }


  if ((input == false) && (output == false)) {
    return false;
  }



# elif defined(ESP32_CLASSIC)

  // ESP32 classic

  // Input GPIOs:  0-19, 21-23, 25-27, 32-39
  // Output GPIOs: 0-19, 21-23, 25-27, 32-33
  input  = gpio <= 39;
  output = gpio <= 33;

  if ((gpio < 0) || (gpio == 20) || (gpio == 24) || ((gpio > 27) && (gpio < 32))) {
    input  = false;
    output = false;
  }

  if ((gpio == 37) || (gpio == 38)) {
    // Pins are not present on the ESP32
    input  = true;
    output = false;
  }

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    // Connected to the integrated SPI flash.
    input   = false;
    output  = false;
    warning = true;
  }

  if ((input == false) && (output == false)) {
    return false;
  }

  // GPIO 0 & 2 can't be used as an input. State during boot is dependent on boot mode.
  if ((gpio == 0) || (gpio == 2)) {
    warning = true;
  }

  if (gpio == 12) {
    // If driven High, flash voltage (VDD_SDIO) is 1.8V not default 3.3V.
    // Has internal pull-down, so unconnected = Low = 3.3V.
    // May prevent flashing and/or booting if 3.3V flash is used and this pin is
    // pulled high, causing the flash to brownout.
    // See the ESP32 datasheet for more details.
    warning = true;
  }

  if (gpio == 15) {
    // If driven Low, silences boot messages printed by the ROM bootloader.
    // Has an internal pull-up, so unconnected = High = normal output.
    warning = true;
  }

  #  if FEATURE_ETHERNET

  // Check pins used for RMII Ethernet PHY
  if (NetworkMedium_t::Ethernet == Settings.NetworkMedium) {
    switch (gpio) {
      case 0:
      case 21:
      case 19:
      case 22:
      case 25:
      case 26:
      case 27:
        warning = true;
        break;
    }


    // FIXME TD-er: Must we also check for pins used for MDC/MDIO and Eth PHY power?
  }


  #  endif // if FEATURE_ETHERNET

  if (FoundPSRAM()) {
    // ESP32 PSRAM can use GPIO 16 and 17
    // There will be a high frequency signal on those pins (flash frequency)
    // which makes them unusable for other purposes.
    // WROVER does not even have these pins made available on the outside.
    switch (gpio) {
      case 16:
      case 17:
        warning = true;
        break;
    }
  }

# else
  static_assert(false, "Implement processor architecture");
# endif 

  return true;
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

# ifdef ESP32S2

  // GPI: GPIO46 is fixed to pull-down and is input only.
  if (gpio <= 45) {
    hasPullUp   = true;
    hasPullDown = true;
  }
# elif defined(ESP32S3)

  if (validGpio(gpio)) {
    hasPullUp   = true;
    hasPullDown = true;
  }

# elif defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)

  if (validGpio(gpio)) {
    hasPullUp   = true;
    hasPullDown = true;
  }

# elif defined(ESP32_CLASSIC)

  // ESP32 classic
  if (gpio >= 34) {
    // For GPIO 34 .. 39, no pull-up nor pull-down.
  } else if (gpio == 12) {
    // No Pull-up on GPIO12
    // compatible with the SDIO protocol.
    // Just connect GPIO12 to VDD via a 10 kOhm resistor.
  } else {
    hasPullUp   = true;
    hasPullDown = true;
  }

# else
    static_assert(false, "Implement processor architecture");
# endif
  return true;
}

#endif // ifdef ESP32

#ifdef ESP8266

// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr  = -1;
  input  = true;
  output = true;

  // GPIO 0, 2 & 15 can't be used as an input. State during boot is dependent on boot mode.
  warning = (gpio == 0 || gpio == 2 || gpio == 15);

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

#endif // ifdef ESP8266

bool validGpio(int gpio) {
  if (gpio < 0) { return false; }
  #ifdef ESP32
  if (!GPIO_IS_VALID_GPIO(gpio)) { return false; }
  #endif
  #ifdef ESP8266
  if (gpio > MAX_GPIO) { return false; }
  #endif
  int pinnr;
  bool input;
  bool output;
  bool warning;

  return getGpioInfo(gpio, pinnr, input, output, warning);
}

bool isSerialConsolePin(int gpio) {
  if (!Settings.UseSerial) { return false; }

#if defined(SOC_RX0) && defined(SOC_TX0)
  return (gpio == SOC_TX0 || gpio == SOC_RX0)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;
#else
#ifdef ESP32S2

  return (gpio == 1 || gpio == 3)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32S3)

  return (gpio == 43 || gpio == 44)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32C3)

  return (gpio == 21 || gpio == 20) 
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;

#elif defined(ESP32_CLASSIC)
  return gpio == 1 || gpio == 3;

#elif defined(ESP8266)
  return gpio == 1 || gpio == 3;

#else // ifdef ESP32S2
  static_assert(false, "Implement processor architecture");
  return false;
#endif // ifdef ESP32S2
#endif
}

#ifdef ESP32

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

# if defined(ESP32S2) || defined(ESP32S3)

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


# elif defined(ESP32C2)
  if (gpio_pin >= 0 && gpio_pin <= 4) {
    adc = 1;
    ch = gpio_pin;
    return true;
  }
  return false;

# elif defined(ESP32C3)

  if (gpio_pin >= 0 && gpio_pin <= 4) {
    adc = 1;
    ch = gpio_pin;
    return true;
  }
  # if ESP_IDF_VERSION_MAJOR >= 5
  // Support for ADC2 has been dropped. 
  # else
  if (gpio_pin == 5) {
    adc = 2;
    ch = 0;
    return true;
  }
  #endif
  return false;

# elif defined(ESP32C6)
  if (gpio_pin >= 0 && gpio_pin <= 6) {
    adc = 1;
    ch = gpio_pin;
    return true;
  }
  return false;

# elif defined(ESP32_CLASSIC)

  // Classic ESP32
  switch (gpio_pin) {
    case -1: adc = 0; break; // Hall effect Sensor
    case 36: adc = 1; ch = 0; break;
    case 37: adc = 1; ch = 1; break;
    case 38: adc = 1; ch = 2; break;
    case 39: adc = 1; ch = 3; break;
    case 32: adc = 1; ch = 4; t = 9; break;
    case 33: adc = 1; ch = 5; t = 8; break;
    case 34: adc = 1; ch = 6; break;
    case 35: adc = 1; ch = 7; break;
    case 4:  adc = 2; ch = 0; t = 0; break;
    case 0:  adc = 2; ch = 1; t = 1; break;
    case 2:  adc = 2; ch = 2; t = 2; break;
    case 15: adc = 2; ch = 3; t = 3; break;
    case 13: adc = 2; ch = 4; t = 4; break;
    case 12: adc = 2; ch = 5; t = 5; break;
    case 14: adc = 2; ch = 6; t = 6; break;
    case 27: adc = 2; ch = 7; t = 7; break;
    case 25: adc = 2; ch = 8; break;
    case 26: adc = 2; ch = 9; break;
    default:
      return false;
  }

# else

  static_assert(false, "Implement processor architecture");


# endif
  return true;
}

int touchPinToGpio(int touch_pin)
{
# if defined(ESP32S2) || defined(ESP32S3)

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

# elif defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)
// No touch pin support


# elif defined(ESP32_CLASSIC)
  // ESP32 classic
  switch (touch_pin) {
    case 0: return T0;
    case 1: return T1;
    case 2: return T2;
    case 3: return T3;
    case 4: return T4;
    case 5: return T5;
    case 6: return T6;
    case 7: return T7;
    case 8: return T8;
    case 9: return T9;
    default:
      break;
  }

# else
    static_assert(false, "Implement processor architecture");
# endif
  return -1;
}

// Get DAC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param dac        Number of DAC unit
bool getDAC_gpio_info(int gpio_pin, int& dac)
{
  switch (gpio_pin) {
    # if (defined(ESP32C3) || defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C6) || defined(ESP32H2))

    // ESP32-C3, ESP32-S3, ESP32-C2, ESP32-C6 and ESP32-H2 don't have a DAC onboard
    # elif defined(ESP32S2)
    case 17: dac = 1; break;
    case 18: dac = 2; break;
    # else // regular ESP32
    case 25: dac = 1; break;
    case 26: dac = 2; break;
    # endif // 
    default:
      return false;
  }
  return true;
}

#endif // ifdef ESP32
