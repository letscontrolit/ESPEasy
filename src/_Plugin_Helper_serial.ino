#include <ESPeasySerial.h>

static String serialHelper_getSerialTypeLabel(ESPeasySerialType::serialtype serType) {
  if (serType == ESPeasySerialType::serialtype::software) return F("SoftwareSerial");
  if (serType == ESPeasySerialType::serialtype::serial0_swap) return F("HW Serial0 swap");
  int portnr = 0;
  if (serType == ESPeasySerialType::serialtype::serial1) portnr = 1;
  if (serType == ESPeasySerialType::serialtype::serial2) portnr = 2;

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

void serialHelper_webformLoad(struct EventStruct *event) {
  html_add_script(F("function serialPortChanged(elem){ var style = elem.value == 0 ? '' : 'none'; document.getElementById('tr_taskdevicepin1').style.display = style; document.getElementById('tr_taskdevicepin2').style.display = style; }"), false);

  String options[NR_ESPEASY_SERIAL_TYPES];
  int ids[NR_ESPEASY_SERIAL_TYPES];

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
      }
      options[index] = option;
      ids[index] = i;
      ++index;
    }
  }
  addFormSelector_script(F("Serial Port"), F("serPort"), NR_ESPEASY_SERIAL_TYPES,
                     options, ids, NULL,
                     static_cast<int>(serialHelper_getSerialType(event)),
                     F("serialPortChanged(this)")); // Script to toggle GPIO visibility when changing selection.
  html_add_script(F("document.getElementById('serPort').onchange();"), false);
}

void serialHelper_webformSave(struct EventStruct *event) {
  int serialPortSelected = getFormItemInt(F("serPort"), 0);
  if (serialPortSelected > 0) {
    int rxPin, txPin;
    ESPeasySerialType::serialtype serType = static_cast<ESPeasySerialType::serialtype>(serialPortSelected);
    if (ESPeasySerialType::getSerialTypePins(serType, rxPin, txPin)) {
      CONFIG_PIN1 = rxPin;
      CONFIG_PIN2 = txPin;
    }
  }
}
