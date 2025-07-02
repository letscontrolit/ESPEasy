#include "../Helpers/StringGenerator_GPIO.h"

#include "../Globals/Settings.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/Hardware_device_info.h"
#include "../Helpers/StringConverter.h"
#include "../../ESPEasy_common.h"

/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
const __FlashStringHelper* formatGpioDirection(gpio_direction direction) {
  switch (direction) {
    case gpio_direction::gpio_input:         return F("&larr; ");
    case gpio_direction::gpio_output:        return F("&rarr; ");
    case gpio_direction::gpio_bidirectional: return F("&#8644; ");
    case gpio_direction::gpio_direction_MAX: break;
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
  if (reserve_special(result, reserveLength)) {
    result += F("GPIO ");
    result += formatGpioDirection(direction);
    result += label;

    if (optional) {
      result += F("(optional)");
    }
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

String formatGpioName_serialTX(bool optional)
{
  return concat(F("ESP TX "), formatGpioName_TX(optional));
}

String formatGpioName_serialRX(bool optional)
{
  return concat(F("ESP RX "), formatGpioName_RX(optional));
}

String formatGpioName_TX_HW(bool optional) {
  return formatGpioName(F("RX (HW)"), gpio_direction::gpio_output, optional);
}

String formatGpioName_RX_HW(bool optional) {
  return formatGpioName(F("TX (HW)"), gpio_direction::gpio_input, optional);
}

#ifdef ESP32

#if SOC_ADC_SUPPORTED
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
  return EMPTY_STRING;
}
#endif

#if SOC_DAC_SUPPORTED
String formatGpioName_DAC(int gpio_pin) {
  int dac;

  if (getDAC_gpio_info(gpio_pin, dac)) {
    return concat(F("DAC"), dac);
  }
  return EMPTY_STRING;
}
#endif


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

#ifdef ESP8266
  if (pinnr >= 0) {
    result += F(" (D");
    result += pinnr;
    result += ')';
  }
#endif

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

const __FlashStringHelper* getConflictingUse(int gpio, PinSelectPurpose purpose, bool ignorePSRAMpins)
{
#ifdef PIN_USB_D_MIN
  if (gpio == PIN_USB_D_MIN) { return F("USB_D-"); }
#endif
#ifdef PIN_USB_D_PLUS
  if (gpio == PIN_USB_D_PLUS) { return F("USB_D+"); }
#endif

  if (isFlashInterfacePin_ESPEasy(gpio)) {
    return F("Flash");
  }

#ifdef ESP32
  if (!ignorePSRAMpins && isPSRAMInterfacePin(gpio)) { // PSRAM pins can be shared with SPI
    return F("PSRAM");
  }
#endif

  # ifdef ESP32S2


  #elif defined(ESP32S3)

  // See Appendix A, page 71: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf

  #elif defined(ESP32C6) 

  if (gpio == 27) {
    // By default VDD_SPI is the power supply pin for embedded flash or external flash. It can only be used as GPIO
    // only when the chip is connected to an external flash, and this flash is powered by an external power supply
    return F("Flash Vdd"); 
  }

  #elif defined(ESP32C2) || defined(ESP32C3) 

  if (gpio == 11) {
    // By default VDD_SPI is the power supply pin for embedded flash or external flash. It can only be used as GPIO11
    // only when the chip is connected to an external flash, and this flash is powered by an external power supply
    return F("Flash Vdd"); 
  }

  # elif defined(ESP32_CLASSIC)

  # elif defined(ESP8266)

  # else
    static_assert(false, "Implement processor architecture");

  # endif 

  bool includeI2C = true;
  bool includeSPI = true;
  #if FEATURE_SD
  bool includeSDCard = true;
  #endif
  bool includeSerial = Settings.UseSerial; // Only need to check if Serial Port Console is enabled

  #if FEATURE_ETHERNET
  bool includeEthernet = true;
  #endif // if FEATURE_ETHERNET
  bool includeStatusLed = true;
  bool includeResetPin = true;

  switch (purpose) {
    case PinSelectPurpose::I2C:
#if FEATURE_I2C_MULTIPLE
    case PinSelectPurpose::I2C_2:
#if FEATURE_I2C_INTERFACE_3
    case PinSelectPurpose::I2C_3:
#endif
#endif
      includeI2C = false;
      break;
    case PinSelectPurpose::SPI:
    case PinSelectPurpose::SPI_MISO:
      includeSPI = false;
      break;
    case PinSelectPurpose::Serial_input:
    case PinSelectPurpose::Serial_output:
      includeSerial = false;
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
    case PinSelectPurpose::DAC:
      break;
    #if FEATURE_SD
    case PinSelectPurpose::SD_Card:
      includeSDCard = false;
      break;
    #endif
    case PinSelectPurpose::Status_led:
      includeStatusLed = false;
      break;
    case PinSelectPurpose::Reset_pin:
      includeResetPin = false;
      break;
  }

  if (includeI2C && Settings.isI2C_pin(gpio)) {
    for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
    {
      if (Settings.getI2CSdaPin(i2cBus) == gpio) {
        #if FEATURE_I2C_MULTIPLE
        switch (i2cBus) {
          case 0:  return F("I2C SDA (bus 0)");
          case 1:  return F("I2C SDA (bus 1)");
          #if FEATURE_I2C_INTERFACE_3
          case 2:  return F("I2C SDA (bus 2)");
          #endif
        }
        #else
        return F("I2C SDA");
        #endif
      }
      if (Settings.getI2CSclPin(i2cBus) == gpio) {
        #if FEATURE_I2C_MULTIPLE
        switch (i2cBus) {
          case 0:  return F("I2C SCL (bus 0)");
          case 1:  return F("I2C SCL (bus 1)");
          #if FEATURE_I2C_INTERFACE_3
          case 2:  return F("I2C SCL (bus 2)");
          #endif
        }
        #else
        return F("I2C SCL");
        #endif
      }
    }
  }

  if (includeSPI && Settings.isSPI_pin(gpio)) {
    return F("SPI");
  }

  if (includeStatusLed && (Settings.Pin_status_led == gpio) && (-1 != gpio)) {
    return F("Wifi Status LED");
  }

  if (includeResetPin && (Settings.Pin_Reset == gpio) && (-1 != gpio)) {
    return F("Reset Pin");
  }

  if (includeSerial) {
    #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    if (Settings.UseSerial && 
        (Settings.console_serial_port == 2  // 2 == ESPEasySerialPort::serial0
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         || Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
        ))
    #else
    if (Settings.UseSerial) 
    #endif
    {
      if (gpio == SOC_TX0) { return F("TX0"); }

      if (gpio == SOC_RX0) { return F("RX0"); }
    }
  }

  #if FEATURE_SD
  if (validGpio(gpio) && Settings.Pin_sd_cs == gpio && includeSDCard) { return F("SD-Card CS"); }
  #endif // if FEATURE_SD


  #if FEATURE_ETHERNET
  if (isSPI_EthernetType(Settings.ETH_Phy_Type)) {
    if (includeEthernet && Settings.isEthernetPinOptional(gpio)) {
      if (Settings.ETH_Pin_mdc_cs == gpio) { return F("Eth SPI CS"); }

      if (Settings.ETH_Pin_mdio_irq == gpio) { return F("Eth SPI IRQ"); }

      if (Settings.ETH_Pin_power_rst == gpio) { return F("Eth SPI RST"); }
    }
  } else {
    if (Settings.isEthernetPin(gpio)) {
      return F("Eth");
    }

    if (includeEthernet && Settings.isEthernetPinOptional(gpio)) {
      if (isGpioUsedInETHClockMode(Settings.ETH_Clock_Mode, gpio)) { return F("Eth Clock"); }

      if (Settings.ETH_Pin_mdc_cs == gpio) { return F("Eth MDC"); }

      if (Settings.ETH_Pin_mdio_irq == gpio) { return F("Eth MDIO"); }

      if (Settings.ETH_Pin_power_rst == gpio) { return F("Eth Pwr"); }

      return F("Eth");
    }
  }
  #endif // if FEATURE_ETHERNET

  if (isBootStrapPin(gpio)) {
    return F("Boot Strapping");
  }

  return F("");
}

String getConflictingUse_wrapped(int gpio, PinSelectPurpose purpose, bool ignorePSRAMpins)
{
  const String conflict = getConflictingUse(gpio, purpose, ignorePSRAMpins);

  if (conflict.isEmpty()) { return conflict; }
  String res = F(" [");

  res += conflict;
  res += ']';
  return res;
}
