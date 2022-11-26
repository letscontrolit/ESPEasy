#include "../PluginStructs/P085_data_struct.h"

#ifdef USES_P085

P085_data_struct::~P085_data_struct() {
  modbus.reset();
}

void P085_data_struct::reset() {
  modbus.reset();
}

bool P085_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, int8_t dere_pin,
                            unsigned int baudrate, uint8_t modbusAddress) {
  return modbus.init(port, serial_rx, serial_tx, baudrate, modbusAddress, dere_pin);
}

bool P085_data_struct::isInitialized() const {
  return modbus.isInitialized();
}

const __FlashStringHelper* Plugin_085_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case P085_QUERY_V:      return displayString ? F("Voltage (V)") : F("V");
    case P085_QUERY_A:      return displayString ? F("Current (A)") : F("A");
    case P085_QUERY_W:      return displayString ? F("Power (W)") : F("W");
    case P085_QUERY_Wh_imp: return displayString ? F("Import Energy (Wh)") : F("Wh_imp");
    case P085_QUERY_Wh_exp: return displayString ? F("Export Energy (Wh)") : F("Wh_exp");
    case P085_QUERY_Wh_tot: return displayString ? F("Total Energy (Wh)") : F("Wh_tot");
    case P085_QUERY_Wh_net: return displayString ? F("Net Energy (Wh)") : F("Wh_net");
    case P085_QUERY_h_tot:  return displayString ? F("Meter Running Time (h)") : F("h_tot");
    case P085_QUERY_h_load: return displayString ? F("Load Running Time (h)") : F("h_load");
  }
  return F("");
}

int p085_storageValueToBaudrate(uint8_t baudrate_setting) {
  switch (baudrate_setting) {
    case 0:
      return 1200;
    case 1:
      return 2400;
    case 2:
      return 4800;
    case 3:
      return 9600;
    case 4:
      return 19200;
    case 5:
      return 38500;
  }
  return 19200;
}

float p085_readValue(uint8_t query, struct EventStruct *event) {
  P085_data_struct *P085_data =
    static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P085_data) && P085_data->isInitialized()) {
    switch (query) {
      case P085_QUERY_V:
        return P085_data->modbus.read_float_HoldingRegister(0x200);
      case P085_QUERY_A:
        return P085_data->modbus.read_float_HoldingRegister(0x202);
      case P085_QUERY_W:
        return P085_data->modbus.read_float_HoldingRegister(0x204) * 1000.0f; // power (kW => W)
      case P085_QUERY_Wh_imp:
        return P085_data->modbus.read_32b_HoldingRegister(0x300) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_exp:
        return P085_data->modbus.read_32b_HoldingRegister(0x302) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_tot:
        return P085_data->modbus.read_32b_HoldingRegister(0x304) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_net:
      {
        int64_t intvalue = P085_data->modbus.read_32b_HoldingRegister(0x306);

        if (intvalue >= 2147483648ll) {
          intvalue = 4294967296ll - intvalue;
        }
        float value = static_cast<float>(intvalue);
        value *= 10.0f; // 0.01 kWh => Wh
        return value;
      }
      case P085_QUERY_h_tot:
        return P085_data->modbus.read_32b_HoldingRegister(0x280) / 100.0f;
      case P085_QUERY_h_load:
        return P085_data->modbus.read_32b_HoldingRegister(0x282) / 100.0f;
    }
  }
  return 0.0f;
}

void p085_showValueLoadPage(uint8_t query, struct EventStruct *event) {
  addRowLabel(Plugin_085_valuename(query, true));
  addHtml(String(p085_readValue(query, event)));
}

#endif // ifdef USES_P085
