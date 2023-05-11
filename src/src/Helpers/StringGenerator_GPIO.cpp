#include "../Helpers/StringGenerator_GPIO.h"

#include "../Globals/Settings.h"
#include "../Helpers/Hardware.h"
#include "../../ESPEasy_common.h"

/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
const __FlashStringHelper* formatGpioDirection(gpio_direction direction) {
  switch (direction) {
    case gpio_direction::gpio_input:         return F("&larr; ");
    case gpio_direction::gpio_output:        return F("&rarr; ");
    case gpio_direction::gpio_bidirectional: return F("&#8644; ");
  }
  return F("");
}

String formatGpioLabel(int gpio, bool includeWarning) {
  int  pinnr = -1;
  bool input, output, warning;

  if (getGpioInfo(gpio, pinnr, input, output, warning)) {
    if (!includeWarning) {
      return createGPIO_label(gpio, pinnr, true, true, false);
    }
    return createGPIO_label(gpio, pinnr, input, output, warning);
  }
  return F("-");
}

String formatGpioName(const __FlashStringHelper * label, gpio_direction direction, bool optional) {
  int reserveLength = 5 /* "GPIO " */ + 8 /* "&#8644; " */ + strlen_P((PGM_P)label);

  if (optional) {
    reserveLength += 11;
  }
  String result;

  result.reserve(reserveLength);
  result += F("GPIO ");
  result += formatGpioDirection(direction);
  result += label;

  if (optional) {
    result += F("(optional)");
  }
  return result;
}

String formatGpioName_input(const __FlashStringHelper * label) {
  return formatGpioName(label, gpio_direction::gpio_input, false);
}

String formatGpioName_output(const __FlashStringHelper * label) {
  return formatGpioName(label, gpio_direction::gpio_output, false);
}

String formatGpioName_bidirectional(const __FlashStringHelper * label) {
  return formatGpioName(label, gpio_direction::gpio_bidirectional, false);
}

String formatGpioName_input_optional(const __FlashStringHelper * label) {
  return formatGpioName(label, gpio_direction::gpio_input, true);
}

String formatGpioName_output_optional(const __FlashStringHelper * label) {
  return formatGpioName(label, gpio_direction::gpio_output, true);
}

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional) {
  return formatGpioName(F("RX"), gpio_direction::gpio_output, optional);
}

String formatGpioName_RX(bool optional) {
  return formatGpioName(F("TX"), gpio_direction::gpio_input, optional);
}

String formatGpioName_TX_HW(bool optional) {
  return formatGpioName(F("RX (HW)"), gpio_direction::gpio_output, optional);
}

String formatGpioName_RX_HW(bool optional) {
  return formatGpioName(F("TX (HW)"), gpio_direction::gpio_input, optional);
}

#ifdef ESP32

String formatGpioName_ADC(int gpio_pin) {
  int adc, ch, t;

  if (getADC_gpio_info(gpio_pin, adc, ch, t)) {
    if (adc == 0) {
      return F("Hall Effect");
    }
    String res = F("ADC# ch?");
    res.replace(F("#"), String(adc));
    res.replace(F("?"), String(ch));

    if (t >= 0) {
      res += F(" (T");
      res += t;
      res += ')';
    }
    return res;
  }
  return "";
}

#endif // ifdef ESP32

// ********************************************************************************
// Add a GPIO pin select dropdown list for 8266, 8285 or ESP32
// ********************************************************************************
String createGPIO_label(int gpio, int pinnr, bool input, bool output, bool warning) {
  if (gpio < 0) { return F("- None -"); }
  String result;

  result.reserve(24);
  result  = F("GPIO-");
  result += gpio;

  if (pinnr >= 0) {
    result += F(" (D");
    result += pinnr;
    result += ')';
  }

  if (input != output) {
    result += ' ';
    result += input ? F(HTML_SYMBOL_INPUT) : F(HTML_SYMBOL_OUTPUT);
  }

  if (warning) {
    result += ' ';
    result += F(HTML_SYMBOL_WARNING);
  }
  return result;
}

const __FlashStringHelper* getConflictingUse(int gpio, PinSelectPurpose purpose)
{

  # ifdef ESP32S2

  if (Settings.UseSerial) {
    if (gpio == 1) { return F("TX0"); }

    if (gpio == 3) { return F("RX0"); }
  }


  if (FoundPSRAM() && (gpio == 26)) {
    // ESP32-S2 PSRAM can use GPIO 26 (and 27..32 but those are always unavailable)
    return F("PSRAM");
  }

  #elif defined(ESP32S3)

  // SPI0/1: GPIO26-32 are usually used for SPI flash and PSRAM and not recommended for other uses. 
  // When using Octal Flash or Octal PSRAM or both, GPIO33~37 are connected to SPIIO4 ~ SPIIO7 and SPIDQS. 
  // Therefore, on boards embedded with ESP32-S3R8 / ESP32-S3R8V chip, GPIO33~37 are also not recommended for other uses.

  if ((gpio >= 26) && (gpio <= 37)) {
    if (FoundPSRAM()) {
      return F("PSRAM");
    } else {
      return F("Flash");
    }
  }

  // See Appendix A, page 71: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf

  if (gpio == 19) { return F("USB_D-"); }
  if (gpio == 20) { return F("USB_D+"); }

  if (Settings.UseSerial) {
    if (gpio == 43) { return F("TX0"); }

    if (gpio == 44) { return F("RX0"); }
  }

  #elif defined(ESP32C3)

  if (Settings.UseSerial) {
    if (gpio == 21) { return F("TX0"); }

    if (gpio == 20) { return F("RX0"); }
  }

  if (gpio == 18) { return F("USB_D-"); }
  if (gpio == 19) { return F("USB_D+"); }
  
  if (gpio == 11) {
    // By default VDD_SPI is the power supply pin for embedded flash or external flash. It can only be used as GPIO11
    // only when the chip is connected to an external flash, and this flash is powered by an external power supply
    return F("Flash Vdd"); 
  }
  if ((gpio >= 12) && (gpio <= 17)) {
    return F("Flash");
  }


  # elif defined(ESP32_CLASSIC)

  if (Settings.UseSerial) {
    if (gpio == 1) { return F("TX0"); }

    if (gpio == 3) { return F("RX0"); }
  }


  if (FoundPSRAM()) {
    // ESP32 PSRAM can use GPIO 16 and 17
    switch (gpio) {
      case 16:
      case 17:
        return F("PSRAM");
    }
  }

  # elif defined(ESP8266)

  if (Settings.UseSerial) {
    if (gpio == 1) { return F("TX0"); }

    if (gpio == 3) { return F("RX0"); }
  }

  # else
    static_assert(false, "Implement processor architecture");

  # endif 

  bool includeI2C = true;
  bool includeSPI = true;

  #if FEATURE_ETHERNET
  bool includeEthernet = true;
  #endif // if FEATURE_ETHERNET

  switch (purpose) {
    case PinSelectPurpose::I2C:
      includeI2C = false;
      break;
    case PinSelectPurpose::SPI:
    case PinSelectPurpose::SPI_MISO:
      includeSPI = false;
      break;
    case PinSelectPurpose::Ethernet:
      #if FEATURE_ETHERNET
      includeEthernet = false;
      #endif // if FEATURE_ETHERNET
      break;
    case PinSelectPurpose::Generic:
    case PinSelectPurpose::Generic_input:
    case PinSelectPurpose::Generic_output:
    case PinSelectPurpose::Generic_bidir:
      break;
  }

  if (includeI2C && Settings.isI2C_pin(gpio)) {
    return (Settings.Pin_i2c_sda == gpio) ?  F("I2C SDA") : F("I2C SCL");
  }

  if (includeSPI && Settings.isSPI_pin(gpio)) {
    return F("SPI");
  }
  #if FEATURE_ETHERNET

  if (Settings.isEthernetPin(gpio)) {
    return F("Eth");
  }

  if (includeEthernet && Settings.isEthernetPinOptional(gpio)) {
    if (isGpioUsedInETHClockMode(Settings.ETH_Clock_Mode, gpio)) { return F("Eth Clock"); }

    if (Settings.ETH_Pin_mdc == gpio) { return F("Eth MDC"); }

    if (Settings.ETH_Pin_mdio == gpio) { return F("Eth MDIO"); }

    if (Settings.ETH_Pin_power == gpio) { return F("Eth Pwr"); }

    return F("Eth");
  }
  #endif // if FEATURE_ETHERNET



  return F("");
}

String getConflictingUse_wrapped(int gpio, PinSelectPurpose purpose)
{
  String conflict = getConflictingUse(gpio, purpose);

  if (conflict.isEmpty()) { return conflict; }
  String res = F(" [");

  res += conflict;
  res += ']';
  return res;
}
