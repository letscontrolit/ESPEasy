#include "StringGenerator_GPIO.h"

#include "../Globals/Settings.h"
#include "../Helpers/Hardware.h"

/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
String formatGpioDirection(gpio_direction direction) {
  switch (direction) {
    case gpio_input:         return F("&larr; ");
    case gpio_output:        return F("&rarr; ");
    case gpio_bidirectional: return F("&#8644; ");
  }
  return "";
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

    bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));

    if (serialPinConflict) {
      if (gpio == 1) { result += F(" TX0"); }

      if (gpio == 3) { result += F(" RX0"); }
    }
  }
  return result;
}
