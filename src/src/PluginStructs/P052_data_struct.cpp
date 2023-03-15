#include "../PluginStructs/P052_data_struct.h"

#ifdef USES_P052

P052_data_struct::~P052_data_struct() {
  modbus.reset();
}

void P052_data_struct::reset() {
  modbus.reset();
}

bool P052_data_struct::init(const ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx) {
  return modbus.init(port, serial_rx, serial_tx, 9600, P052_MODBUS_SLAVE_ADDRESS);
}

bool P052_data_struct::isInitialized() const {
  return modbus.isInitialized();
}

const __FlashStringHelper * P052_data_struct::Plugin_052_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case 0:  return displayString ? F("Empty") : F("");
    case 1:  return displayString ? F("Carbon Dioxide") : F("co2");
    case 2:  return displayString ? F("Temperature") : F("T");
    case 3:  return displayString ? F("Humidity") : F("H");
    case 4:  return displayString ? F("Relay Status") : F("rel");
    case 5:  return displayString ? F("Temperature Adjustment") : F("Tadj");
    case 6:  return displayString ? F("ABC period") : F("abc_per");
    case 7:  return displayString ? F("Error Status") : F("err");
    default:
      break;
  }
  return F("");
}

#endif // ifdef USES_P052
