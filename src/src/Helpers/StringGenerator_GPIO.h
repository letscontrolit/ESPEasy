#ifndef HELPERS_STRINGGENERATOR_GPIO_H
#define HELPERS_STRINGGENERATOR_GPIO_H


#include "../../ESPEasy_common.h"

#define HTML_SYMBOL_WARNING "&#9888;"
#define HTML_SYMBOL_INPUT   "&#8656;"
#define HTML_SYMBOL_OUTPUT  "&#8658;"
#define HTML_SYMBOL_I_O     "&#8660;"

#define GPIO_DIRECTION_NR_BITS 2
enum class gpio_direction : uint8_t {
  gpio_input,
  gpio_output,
  gpio_bidirectional,
  gpio_direction_MAX // Keep last, used as bit-size value and dummy-default value
};

enum class PinSelectPurpose : uint8_t {
  Generic,
  Generic_input,
  Generic_output,
  Generic_bidir,
  I2C,
  #if FEATURE_I2C_MULTIPLE
  I2C_2,
  #if FEATURE_I2C_INTERFACE_3
  I2C_3,
  #endif
  #endif
  SPI,
  SPI_MISO,
  Ethernet,
  Serial_input,
  Serial_output,
  DAC,
  #if FEATURE_SD
  SD_Card,
  #endif
  Status_led,
  Reset_pin,
};




/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
const __FlashStringHelper * formatGpioDirection(gpio_direction direction);

String formatGpioLabel(int  gpio,
                       bool includeWarning);

String formatGpioName(const __FlashStringHelper * label,
                      gpio_direction direction,
                      bool           optional = false);

String formatGpioName_input(const __FlashStringHelper * label);
String formatGpioName_output(const __FlashStringHelper * label);
String formatGpioName_bidirectional(const __FlashStringHelper * label);
String formatGpioName_input_optional(const __FlashStringHelper * label);

String formatGpioName_output_optional(const __FlashStringHelper * label);

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional);

String formatGpioName_RX(bool optional);

String formatGpioName_serialTX(bool optional);

String formatGpioName_serialRX(bool optional);

String formatGpioName_TX_HW(bool optional);

String formatGpioName_RX_HW(bool optional);

#ifdef ESP32

#if SOC_ADC_SUPPORTED
String formatGpioName_ADC(int gpio_pin);
#endif

#if SOC_DAC_SUPPORTED
String formatGpioName_DAC(int gpio_pin);
#endif

#endif // ifdef ESP32

String createGPIO_label(int  gpio,
                        int  pinnr,
                        bool input,
                        bool output,
                        bool warning);

const __FlashStringHelper * getConflictingUse(int gpio, PinSelectPurpose purpose = PinSelectPurpose::Generic, bool ignorePSRAMpin = false);

String getConflictingUse_wrapped(int gpio, PinSelectPurpose purpose = PinSelectPurpose::Generic, bool ignorePSRAMpin = false);


#endif