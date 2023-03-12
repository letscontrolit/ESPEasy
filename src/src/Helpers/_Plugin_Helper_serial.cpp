#include "../Helpers/_Plugin_Helper_serial.h"


#include "../../_Plugin_Helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Cache.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"

#include <ESPEasySerialType.h>


String serialHelper_getSerialTypeLabel(ESPEasySerialPort serType) {
  return ESPEasySerialPort_toString(serType);
}

void serialHelper_log_GpioDescription(ESPEasySerialPort typeHint, int config_pin1, int config_pin2) {
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("Serial : ");
    log += serialHelper_getGpioDescription(typeHint, config_pin1, config_pin2, " ");
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  #endif // ifndef BUILD_NO_DEBUG
}

String serialHelper_getGpioDescription(ESPEasySerialPort typeHint, int config_pin1, int config_pin2, const String& newline) {
  String result;

  result.reserve(20);

  switch (ESPeasySerialType::getSerialType(typeHint, config_pin1, config_pin2)) {
    case ESPEasySerialPort::sc16is752:
    {
      result += formatToHex(config_pin1);
      result += newline;
      result += F(" ch: ");
      result += config_pin2 == 0 ? F("A") : F("B");
      return result;
    }
    case ESPEasySerialPort::software:
    case ESPEasySerialPort::serial0_swap:
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial1:
    case ESPEasySerialPort::serial2:
    {
      result += F("RX: ");
      result += formatGpioLabel(config_pin1, false);
      result += newline;
      result += F("TX: ");
      result += formatGpioLabel(config_pin2, false);
      break;
    }
    default:
      break;
  }
  return result;
}

void serialHelper_getGpioNames(struct EventStruct *event, bool rxOptional, bool txOptional) {
  event->String1 = formatGpioName_RX(rxOptional);
  event->String2 = formatGpioName_TX(txOptional);
}

int8_t serialHelper_getRxPin(struct EventStruct *event) {
  return CONFIG_PIN1;
}

int8_t serialHelper_getTxPin(struct EventStruct *event) {
  return CONFIG_PIN2;
}

ESPEasySerialPort serialHelper_getSerialType(struct EventStruct *event) {
  ESPEasySerialPort serialType = static_cast<ESPEasySerialPort>(CONFIG_PORT);

  if (serialType != ESPEasySerialPort::not_set) {
    return serialType;
  }

  return ESPeasySerialType::getSerialType(
    serialType,
    serialHelper_getRxPin(event),
    serialHelper_getTxPin(event));
}

String serialHelper_getSerialTypeLabel(struct EventStruct *event) {
  return serialHelper_getSerialTypeLabel(serialHelper_getSerialType(event));
}

#ifndef DISABLE_SC16IS752_Serial
void serialHelper_addI2CuartSelectors(int address, int channel) {
  # define     SC16IS752_I2C_ADDRESSES             16
  # define     SC16IS752_I2C_BASE_ADDR             (0x90 >> 1)
  # define     SC16IS752_CHANNELS                  2
  # define     SC16IS752_CHANNEL_A                 0x00
  # define     SC16IS752_CHANNEL_B                 0x01
  {
    String id = F("i2cuart_addr");
    addRowLabel_tr_id(F("I2C Address"), id);
    do_addSelector_Head(id, F(""), EMPTY_STRING, false);

    if ((address < SC16IS752_I2C_BASE_ADDR) || (address >= (SC16IS752_I2C_BASE_ADDR + SC16IS752_I2C_ADDRESSES))) {
      // selected address is not in range
      address = SC16IS752_I2C_BASE_ADDR;
    }

    for (int i = 0; i < SC16IS752_I2C_ADDRESSES; i++)
    {
      int addr = SC16IS752_I2C_BASE_ADDR + i;
      String option;
      option.reserve(24);
      option  = formatToHex(addr);
      option += F(" (datasheet: ");
      option += formatToHex(addr * 2);
      option += ')';
      addSelector_Item(option, addr, addr == address);
    }
    addSelector_Foot();
  }
  {
    if ((channel != SC16IS752_CHANNEL_A) && (channel != SC16IS752_CHANNEL_B)) {
      channel = SC16IS752_CHANNEL_A;
    }
    const __FlashStringHelper *chOptions[SC16IS752_CHANNELS] = {
      F("A"),
      F("B"),
    };
    const int chValues[SC16IS752_CHANNELS] = {
      SC16IS752_CHANNEL_A,
      SC16IS752_CHANNEL_B,
    };
    addFormSelector(F("Channel"), F("i2cuart_ch"), SC16IS752_CHANNELS, chOptions, chValues, channel);
  }
}

#endif // ifndef DISABLE_SC16IS752_Serial

void serialHelper_webformLoad(struct EventStruct *event) {
  serialHelper_webformLoad(event, true);
}

// These helper functions were made to create a generic interface to setup serial port config.
// See issue #2343 and Pull request https://github.com/letscontrolit/ESPEasy/pull/2352
// For now P020 and P044 have been reverted to make them work again.
void serialHelper_webformLoad(struct EventStruct *event, bool allowSoftwareSerial) {
  serialHelper_webformLoad(static_cast<ESPEasySerialPort>(CONFIG_PORT),
                           serialHelper_getRxPin(event),
                           serialHelper_getTxPin(event),
                           allowSoftwareSerial);
}

void serialHelper_webformLoad(ESPEasySerialPort port, int rxPinDef, int txPinDef, bool allowSoftwareSerial) {
  // Field for I2C addr & RX are shared
  // Field for channel and TX are shared
  #ifdef ESP8266

  // Script to show GPIO pins for SoftwareSerial or I2C addresses for the I2C to UART bridge
  // "function serialPortChanged(elem) {var style = elem.value == 6 ? '' : 'none';var i2cstyle = elem.value == 1 ? '' :
  // 'none';document.getElementById('tr_taskdevicepin1').style.display = style;document.getElementById('tr_taskdevicepin2').style.display =
  // style;document.getElementById('tr_i2cuart_addr').style.display = i2cstyle;document.getElementById('tr_i2cuart_ch').style.display =
  // i2cstyle;}"),
  html_add_script(F("function serialPortChanged(e){var t=6==e.value?'':'none',l=1==e.value?'':'none';"
                    "document.getElementById('tr_taskdevicepin1').style.display=t,"
                    "document.getElementById('tr_taskdevicepin2').style.display=t,"
                    "document.getElementById('tr_i2cuart_addr').style.display=l,"
                    "document.getElementById('tr_i2cuart_ch').style.display=l}"),
                  false);
  #endif // ifdef ESP8266
  #ifdef ESP32

  // Script to show GPIO pins for HW serial ports or I2C addresses for the I2C to UART bridge
  // "function serialPortChanged(elem) {var style = (elem.value == 2 || elem.value == 4 || elem.value == 5) ? '' : 'none';var i2cstyle =
  // elem.value == 1 ? '' : 'none';document.getElementById('tr_taskdevicepin1').style.display =
  // style;document.getElementById('tr_taskdevicepin2').style.display = style;document.getElementById('tr_i2cuart_addr').style.display =
  // i2cstyle;document.getElementById('tr_i2cuart_ch').style.display = i2cstyle;}"),
  html_add_script(F("function serialPortChanged(e){var t=2==e.value||4==e.value||5==e.value?'':'none',l=1==e.value?'':'none';"
                    "document.getElementById('tr_taskdevicepin1').style.display=t,"
                    "document.getElementById('tr_taskdevicepin2').style.display=t,"
                    "document.getElementById('tr_i2cuart_addr').style.display=l,"
                    "document.getElementById('tr_i2cuart_ch').style.display=l}"),
                  false);
  #endif // ifdef ESP32

  String options[NR_ESPEASY_SERIAL_TYPES];
  int    ids[NR_ESPEASY_SERIAL_TYPES];
  String attr[NR_ESPEASY_SERIAL_TYPES];

  #ifndef DISABLE_SC16IS752_Serial
  int index = NR_ESPEASY_SERIAL_TYPES - 1; // Place I2C Serial at the end
  #else // ifndef DISABLE_SC16IS752_Serial
  int index = 0;
  #endif // ifndef DISABLE_SC16IS752_Serial

  for (int i = 0; (index < NR_ESPEASY_SERIAL_TYPES) && (i < static_cast<int>(ESPEasySerialPort::MAX_SERIAL_TYPE)); ++i) {
    int rxPin, txPin;
    ESPEasySerialPort serType = static_cast<ESPEasySerialPort>(i);

    if (ESPeasySerialType::getSerialTypePins(serType, rxPin, txPin)) {
      String option;
      option.reserve(48);
      option = serialHelper_getSerialTypeLabel(serType);

      switch (serType) {
        #ifndef DISABLE_SOFTWARE_SERIAL
        case ESPEasySerialPort::software:
        {
          if (!allowSoftwareSerial) {
            attr[index] = F("disabled");
          }
          break;
        }
        #endif // ifndef DISABLE_SOFTWARE_SERIAL
        #ifndef DISABLE_SC16IS752_Serial
        case ESPEasySerialPort::sc16is752:
        {
          break;
        }
        #endif // ifndef DISABLE_SC16IS752_Serial
        case ESPEasySerialPort::serial0:
        case ESPEasySerialPort::serial0_swap:
        case ESPEasySerialPort::serial1:
        case ESPEasySerialPort::serial2:
        {
          #ifdef ESP8266

          // Show pins for ports with fixed pins
          option += F(": ");
          option += formatGpioLabel(rxPin, false);
          option += ' ';
          option += formatGpioDirection(gpio_direction::gpio_input);
          option += F("TX / ");
          option += formatGpioLabel(txPin, false);
          option += ' ';
          option += formatGpioDirection(gpio_direction::gpio_output);
          option += F("RX");
          #endif // ifdef ESP8266
          break;
        }

        default:
          break;
      }
      options[index] = option;
      ids[index]     = i;
      ++index;
      #ifndef DISABLE_SC16IS752_Serial

      if (index == NR_ESPEASY_SERIAL_TYPES) { index = 0; } // Restart at begin of list
      #endif // ifndef DISABLE_SC16IS752_Serial
    }
  }
  addFormSelector_script(F("Serial Port"), F("serPort"), NR_ESPEASY_SERIAL_TYPES,
                         options, ids, nullptr,
                         static_cast<int>(ESPeasySerialType::getSerialType(port, rxPinDef, txPinDef)),
                         F("serialPortChanged(this)")); // Script to toggle GPIO visibility when changing selection.
#ifndef DISABLE_SC16IS752_Serial
  serialHelper_addI2CuartSelectors(rxPinDef, txPinDef);
#endif // ifndef DISABLE_SC16IS752_Serial

#ifdef ESP8266

  if ((rxPinDef == 15) || (txPinDef == 15)) {
    addFormNote(F("GPIO-15 (D8) requires a Buffer Circuit (PNP transistor) or ESP boot may fail."));
  }
#endif // ifdef ESP8266
}

void serialHelper_webformSave(uint8_t& port, int8_t& rxPin, int8_t& txPin) {
  int serialPortSelected = getFormItemInt(F("serPort"), -1);

  if (serialPortSelected < 0) { return; }

  ESPEasySerialPort serType = static_cast<ESPEasySerialPort>(serialPortSelected);

  port = serialPortSelected;

  switch (serType) {
    case ESPEasySerialPort::software:
      break;
    #ifndef DISABLE_SC16IS752_Serial
    case ESPEasySerialPort::sc16is752:
      rxPin = getFormItemInt(F("i2cuart_addr"), rxPin);
      txPin = getFormItemInt(F("i2cuart_ch"), txPin);
      break;
    #endif // ifndef DISABLE_SC16IS752_Serial
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial0_swap:
    case ESPEasySerialPort::serial1:
    case ESPEasySerialPort::serial2:
    {
      #ifdef ESP8266

      // Ports with a fixed pin layout, so load the defaults.
      int tmprxPin, tmptxPin;

      if (ESPeasySerialType::getSerialTypePins(serType, tmprxPin, tmptxPin)) {
        rxPin = tmprxPin;
        txPin = tmptxPin;
      }
      #endif // ifdef ESP8266
      break;
    }
    default:
      break;
  }
}

void serialHelper_webformSave(struct EventStruct *event) {
  serialHelper_webformSave(CONFIG_PORT, CONFIG_PIN1, CONFIG_PIN2);
}

bool serialHelper_isValid_serialconfig(uint8_t serialconfig) {
  if ((serialconfig >= 0x10) && (serialconfig <= 0x3f)) {
    return true;
  }
  return false;
}

void serialHelper_serialconfig_webformLoad(struct EventStruct *event, uint8_t currentSelection) {
  // nrOptions = 4 * 3 * 2  = 24  (bits 5..8 , parity N/E/O  , stopbits 1/2)
  String id = F("serConf");

  addRowLabel_tr_id(F("Serial Config"), id);
  do_addSelector_Head(id, F(""), EMPTY_STRING, false);

  if (currentSelection == 0) {
    // Must truncate it to 1 uint8_t, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
    currentSelection = static_cast<uint8_t>(SERIAL_8N1 & 0xFF); // Some default
  }

  for (uint8_t parity = 0; parity < 3; ++parity) {
    for (uint8_t stopBits = 1; stopBits <= 2; ++stopBits) {
      for (uint8_t bits = 5; bits <= 8; ++bits) {
        String label;
        label.reserve(36);
        label  = String(bits);
        label += F(" bit / parity: ");
        int value = ((bits - 5) << 2);

        switch (parity) {
          case 0: label += F("None"); break;
          case 1: label += F("Even"); value += 2; break;
          case 2: label += F("Odd");  value += 3; break;
        }
        label += F(" / stop bits: ");
        label += String(stopBits);

        // There are also values for 0 and "1.5" stop bit, not used now.
        switch (stopBits) {
          case 1:  value += 0x10; break;
          case 2:  value += 0x30; break;
        }
        addSelector_Item(label, value, value == currentSelection);
      }
    }
  }
  addSelector_Foot();
}

uint8_t serialHelper_serialconfig_webformSave() {
  int serialConfSelected = getFormItemInt(F("serConf"), 0);

  if (serialHelper_isValid_serialconfig(serialConfSelected)) {
    return serialConfSelected;
  }

  // Must truncate it to 1 uint8_t, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
  return static_cast<uint8_t>(SERIAL_8N1 & 0xFF); // Some default
}

// Used by some plugins, which used several TaskDevicePluginConfigLong
uint8_t serialHelper_convertOldSerialConfig(uint8_t newLocationConfig) {
  if (serialHelper_isValid_serialconfig(newLocationConfig)) {
    return newLocationConfig;
  }
  uint8_t serialconfig = 0x10;                                                // Default stopbits = 1

  serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[3];            // Parity
  serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[2] - 5) << 2; // databits

  if (ExtraTaskSettings.TaskDevicePluginConfigLong[4] == 2) {
    serialconfig += 0x20;                                                     // Stopbits = 2
  }

  if (serialHelper_isValid_serialconfig(serialconfig)) {
    return serialconfig;
  }

  // Must truncate it to 1 uint8_t, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
  return static_cast<uint8_t>(SERIAL_8N1 & 0xFF); // Some default
}
