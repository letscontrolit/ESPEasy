#include "ESPeasySerial.h"

struct ESPeasySerialType;

static String serialHelper_getSerialTypeLabel(ESPeasySerialType::serialtype serType) {
  int portnr = 0;
  switch (serType) {
    case ESPeasySerialType::serialtype::software:        return F("SoftwareSerial"); 
    case ESPeasySerialType::serialtype::serial0_swap:    return F("HW Serial0 swap");
    case ESPeasySerialType::serialtype::serial0:         portnr = 0; break;
    case ESPeasySerialType::serialtype::serial1:         portnr = 1; break;
    case ESPeasySerialType::serialtype::serial2:         portnr = 2; break;
    default:
      return "";
  }
  String label = F("HW Serial");
  label += portnr;
  return label;
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

ESPeasySerialType::serialtype serialHelper_getSerialType(struct EventStruct *event) {
  return ESPeasySerialType::getSerialType(
    serialHelper_getRxPin(event),
    serialHelper_getTxPin(event));
}

String serialHelper_getSerialTypeLabel(struct EventStruct *event) {
  return serialHelper_getSerialTypeLabel(serialHelper_getSerialType(event));
}

void serialHelper_webformLoad(struct EventStruct *event) {
  serialHelper_webformLoad(event, true);
}

// These helper functions were made to create a generic interface to setup serial port config.
// See issue #2343 and Pull request https://github.com/letscontrolit/ESPEasy/pull/2352
// For now P020 and P044 have been reverted to make them work again.
void serialHelper_webformLoad(struct EventStruct *event, bool allowSoftwareSerial) {
  serialHelper_webformLoad(serialHelper_getRxPin(event), serialHelper_getTxPin(event), allowSoftwareSerial);
}

void serialHelper_webformLoad(int rxPinDef, int txPinDef, bool allowSoftwareSerial) {
  html_add_script(F(
                    "function serialPortChanged(elem){ var style = elem.value == 0 ? '' : 'none'; document.getElementById('tr_taskdevicepin1').style.display = style; document.getElementById('tr_taskdevicepin2').style.display = style; }"),
                  false);

  String options[NR_ESPEASY_SERIAL_TYPES];
  int    ids[NR_ESPEASY_SERIAL_TYPES];
  String attr[NR_ESPEASY_SERIAL_TYPES];

  int index = 0;

  for (int i = 0; (index < NR_ESPEASY_SERIAL_TYPES) && (i < ESPeasySerialType::serialtype::MAX_SERIAL_TYPE); ++i) {
    int rxPin, txPin;
    ESPeasySerialType::serialtype serType = static_cast<ESPeasySerialType::serialtype>(i);

    if (ESPeasySerialType::getSerialTypePins(serType, rxPin, txPin)) {
      String option;
      option.reserve(48);
      option = serialHelper_getSerialTypeLabel(serType);

      if (serType != ESPeasySerialType::serialtype::software) {
        option += ": ";
        option += formatGpioLabel(rxPin, false);
        option += ' ';
        option += formatGpioDirection(gpio_input);
        option += "TX / ";
        option += formatGpioLabel(txPin, false);
        option += ' ';
        option += formatGpioDirection(gpio_output);
        option += "RX";
      } else {
        if (!allowSoftwareSerial) {
          attr[i] = F("disabled");
        }
      }
      options[index] = option;
      ids[index]     = i;
      ++index;
    }
  }
  addFormSelector_script(F("Serial Port"), F("serPort"), NR_ESPEASY_SERIAL_TYPES,
                         options, ids, NULL,
                         static_cast<int>(ESPeasySerialType::getSerialType(rxPinDef, txPinDef)),
                         F("serialPortChanged(this)")); // Script to toggle GPIO visibility when changing selection.
  html_add_script(F("document.getElementById('serPort').onchange();"), false);

  if (Settings.UseSerial) {
    addFormNote(F("Do <b>NOT</b> combine HW Serial0 and log to serial on Tools->Advanced->Serial Port."));
  }

  if ((rxPinDef == 15) || (txPinDef == 15)) {
    addFormNote(F("GPIO-15 (D8) requires a Buffer Circuit (PNP transistor) or ESP boot may fail."));
  }
}

void serialHelper_webformSave(int8_t &rxPin, int8_t &txPin) {
  int serialPortSelected = getFormItemInt(F("serPort"), 0);

  if (serialPortSelected > 0) {
    int tmprxPin, tmptxPin;
    ESPeasySerialType::serialtype serType = static_cast<ESPeasySerialType::serialtype>(serialPortSelected);

    if (ESPeasySerialType::getSerialTypePins(serType, tmprxPin, tmptxPin)) {
      rxPin = tmprxPin;
      txPin = tmptxPin;
    }
  }
}

void serialHelper_webformSave(struct EventStruct *event) {
  serialHelper_webformSave(CONFIG_PIN1, CONFIG_PIN2);
}

void serialHelper_plugin_init(struct EventStruct *event) {
  ESPeasySerialType::serialtype serType = serialHelper_getSerialType(event);

  if (serType == ESPeasySerialType::serialtype::serial0) {
    Settings.UseSerial = false; // Disable global Serial port.
  }
  #ifdef ESP8266
  if (serType == ESPeasySerialType::serialtype::serial0_swap) {
    Settings.UseSerial = false; // Disable global Serial port.
  }
  #endif
}

bool serialHelper_isValid_serialconfig(byte serialconfig) {
  if ((serialconfig >= 0x10) && (serialconfig <= 0x3f)) {
    return true;
  }
  return false;
}

void serialHelper_serialconfig_webformLoad(struct EventStruct *event, byte currentSelection) {
  // nrOptions = 4 * 3 * 2  = 24  (bits 5..8 , parity N/E/O  , stopbits 1/2)
  String options[24];
  int    values[24];
  byte   index = 0;

  for (byte parity = 0; parity < 3; ++parity) {
    for (byte stopBits = 1; stopBits <= 2; ++stopBits) {
      for (byte bits = 5; bits <= 8; ++bits) {
        String label;
        label.reserve(36);
        label  = String(bits);
        label += F(" bit / parity: ");
        int value = ((bits - 5) << 2);

        switch (parity) {
          case 0: label += "None"; break;
          case 1: label += "Even"; value += 2; break;
          case 2: label += "Odd";  value += 3; break;
        }
        label += F(" / stop bits: ");
        label += String(stopBits);

        // There are also values for 0 and "1.5" stop bit, not used now.
        switch (stopBits) {
          case 1:  value += 0x10; break;
          case 2:  value += 0x30; break;
        }
        options[index] = label;
        values[index]  = value;
        ++index;
      }
    }
  }

  if (currentSelection == 0) {
    // Must truncate it to 1 byte, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
    currentSelection = static_cast<byte>(SERIAL_8N1 & 0xFF); // Some default
  }
  addFormSelector(F("Serial Config"), F("serConf"), 24,
                  options, values, currentSelection);
}

byte serialHelper_serialconfig_webformSave() {
  int serialConfSelected = getFormItemInt(F("serConf"), 0);

  if (serialHelper_isValid_serialconfig(serialConfSelected)) {
    return serialConfSelected;
  }

  // Must truncate it to 1 byte, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
  return static_cast<byte>(SERIAL_8N1 & 0xFF); // Some default
}

// Used by some plugins, which used several TaskDevicePluginConfigLong
byte serialHelper_convertOldSerialConfig(byte newLocationConfig) {
  if (serialHelper_isValid_serialconfig(newLocationConfig)) {
    return newLocationConfig;
  }
  byte serialconfig = 0x10;                                                   // Default stopbits = 1
  serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[3];            // Parity
  serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[2] - 5) << 2; // databits

  if (ExtraTaskSettings.TaskDevicePluginConfigLong[4] == 2) {
    serialconfig += 0x20;                                                     // Stopbits = 2
  }

  if (serialHelper_isValid_serialconfig(serialconfig)) {
    return serialconfig;
  }

  // Must truncate it to 1 byte, since ESP32 uses a 32-bit value. We add these high bits later for ESP32.
  return static_cast<byte>(SERIAL_8N1 & 0xFF); // Some default
}
