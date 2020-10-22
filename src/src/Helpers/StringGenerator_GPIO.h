#ifndef HELPERS_STRINGGENERATOR_GPIO_H
#define HELPERS_STRINGGENERATOR_GPIO_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

#define HTML_SYMBOL_WARNING "&#9888;"
#define HTML_SYMBOL_INPUT   "&#8656;"
#define HTML_SYMBOL_OUTPUT  "&#8658;"
#define HTML_SYMBOL_I_O     "&#8660;"


enum gpio_direction {
  gpio_input,
  gpio_output,
  gpio_bidirectional
};


/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
String formatGpioDirection(gpio_direction direction);

String formatGpioLabel(int  gpio,
                       bool includeWarning);

String formatGpioName(const String & label,
                      gpio_direction direction,
                      bool           optional);

String formatGpioName(const String & label,
                      gpio_direction direction);

String formatGpioName_input(const String& label);
String formatGpioName_output(const String& label);
String formatGpioName_bidirectional(const String& label);
String formatGpioName_input_optional(const String& label);

String formatGpioName_output_optional(const String& label);

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional);

String formatGpioName_RX(bool optional);

String formatGpioName_TX_HW(bool optional);

String formatGpioName_RX_HW(bool optional);

#ifdef ESP32

String formatGpioName_ADC(int gpio_pin);

#endif // ifdef ESP32

String createGPIO_label(int  gpio,
                        int  pinnr,
                        bool input,
                        bool output,
                        bool warning);



#endif