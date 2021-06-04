#include "StringGenerator_GPIO.h"

#include "../Globals/Settings.h"
#include "../Helpers/Hardware.h"
#include "../../ESPEasy_common.h"

/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
const __FlashStringHelper* formatGpioDirection(gpio_direction direction) {
  switch (direction) {
    case gpio_input:         return F("&larr; ");
    case gpio_output:        return F("&rarr; ");
    case gpio_bidirectional: return F("&#8644; ");
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
  return "-";
}

String formatGpioName(const String& label, gpio_direction direction, bool optional) {
  int reserveLength = 5 /* "GPIO " */ + 8 /* "&#8644; " */ + label.length();

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

String formatGpioName(const String& label, gpio_direction direction) {
  return formatGpioName(label, direction, false);
}

String formatGpioName_input(const String& label) {
  return formatGpioName(label, gpio_input, false);
}

String formatGpioName_output(const String& label) {
  return formatGpioName(label, gpio_output, false);
}

String formatGpioName_bidirectional(const String& label) {
  return formatGpioName(label, gpio_bidirectional, false);
}

String formatGpioName_input_optional(const String& label) {
  return formatGpioName(label, gpio_input, true);
}

String formatGpioName_output_optional(const String& label) {
  return formatGpioName(label, gpio_output, true);
}

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional) {
  return formatGpioName(F("RX"), gpio_output, optional);
}

String formatGpioName_RX(bool optional) {
  return formatGpioName(F("TX"), gpio_input, optional);
}

String formatGpioName_TX_HW(bool optional) {
  return formatGpioName(F("RX (HW)"), gpio_output, optional);
}

String formatGpioName_RX_HW(bool optional) {
  return formatGpioName(F("TX (HW)"), gpio_input, optional);
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
  if (Settings.UseSerial) {
    if (gpio == 1) { return F("TX0"); }

    if (gpio == 3) { return F("RX0"); }
  }
  bool includeI2C = true;
  bool includeSPI = true;

  #ifdef HAS_ETHERNET
  bool includeEthernet = true;
  #endif // ifdef HAS_ETHERNET

  switch (purpose) {
    case PinSelectPurpose::I2C:
      includeI2C = false;
      break;
    case PinSelectPurpose::SPI:
      includeSPI = false;
      break;
    case PinSelectPurpose::Ethernet:
      #ifdef HAS_ETHERNET
      includeEthernet = false;
      #endif // ifdef HAS_ETHERNET
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
  #ifdef HAS_ETHERNET

  if (Settings.isEthernetPin(gpio)) {
    return F("Eth");
  }

  if (includeEthernet && Settings.isEthernetPinOptional(gpio)) {
    if (Settings.ETH_Pin_mdc == gpio) { return F("Eth MDC"); }

    if (Settings.ETH_Pin_mdio == gpio) { return F("Eth MDIO"); }

    if (Settings.ETH_Pin_power == gpio) { return F("Eth Pwr"); }

    return F("Eth");
  }
  #endif // ifdef HAS_ETHERNET
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
