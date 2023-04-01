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
  const __FlashStringHelper* strings[] {
    F("Empty"),                  F(""),
    F("Carbon Dioxide"),         F("co2"),
    F("Temperature"),            F("T"),
    F("Humidity"),               F("H"),
    F("Relay Status"),           F("rel"),
    F("Temperature Adjustment"), F("Tadj"),
    F("ABC period"),             F("abc_per"),
    F("Error Status"),           F("err")
  };
  const size_t index = (2* value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = sizeof(strings) / sizeof(strings[0]);
  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

#endif // ifdef USES_P052
