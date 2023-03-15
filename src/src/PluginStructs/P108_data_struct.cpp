#include "../PluginStructs/P108_data_struct.h"

#ifdef USES_P108


P108_data_struct::~P108_data_struct() {
  modbus.reset();
}

void P108_data_struct::reset() {
  modbus.reset();
}

bool P108_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, int8_t dere_pin,
                            unsigned int baudrate, uint8_t modbusAddress) {
  return modbus.init(port, serial_rx, serial_tx, baudrate, modbusAddress, dere_pin);
}

bool P108_data_struct::isInitialized() const {
  return modbus.isInitialized();
}

const __FlashStringHelper* Plugin_108_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case P108_QUERY_V: return displayString ? F("Voltage (V)") : F("V");
    case P108_QUERY_A: return displayString ? F("Current (A)") : F("A");
    case P108_QUERY_W: return displayString ? F("Active Power (W)") : F("W");
    case P108_QUERY_VA: return displayString ? F("Reactive Power (VA)") : F("VA");
    case P108_QUERY_PF: return displayString ? F("Power Factor (Pf)") : F("Pf");
    case P108_QUERY_F: return displayString ? F("Frequency (Hz)") : F("Hz");
    case P108_QUERY_Wh_imp: return displayString ? F("Import Energy (Wh)") : F("Wh_imp");
    case P108_QUERY_Wh_exp: return displayString ? F("Export Energy (Wh)") : F("Wh_exp");
    case P108_QUERY_Wh_tot: return displayString ? F("Total Energy (Wh)") : F("Wh_tot");
  }
  return F("");
}

int p108_storageValueToBaudrate(uint8_t baudrate_setting) {
  switch (baudrate_setting) {
    case 0:
      return 1200;
    case 1:
      return 2400;
    case 2:
      return 4800;
    case 3:
      return 9600;
  }
  return 9600;
}

float p108_readValue(uint8_t query, struct EventStruct *event) {
  uint8_t errorcode           = -1;   // DF - not present in P085
  float   value               = 0.0f; // DF - not present in P085
  P108_data_struct *P108_data =
    static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P108_data) && P108_data->isInitialized()) {
    switch (query) {
      case P108_QUERY_V:
        value = P108_data->modbus.readHoldingRegister(0x0C, errorcode) / 10.0f;  // 0.1 V => V
        break;
      case P108_QUERY_A:
        value = P108_data->modbus.readHoldingRegister(0x0D, errorcode) / 100.0f; // 0.01 A => A
        break;
      case P108_QUERY_W:
        value =  P108_data->modbus.readHoldingRegister(0x0E, errorcode);

        if (value > 32767) { value -= 65535; }
        break;
      case P108_QUERY_VA:
        value = P108_data->modbus.readHoldingRegister(0x0F, errorcode);

        if (value > 32767) { value -= 65535; }
        break;
      case P108_QUERY_PF:
        value = P108_data->modbus.readHoldingRegister(0x10, errorcode) / 1000.0f; // 0.001 Pf => Pf
        break;
      case P108_QUERY_F:
        value = P108_data->modbus.readHoldingRegister(0x11, errorcode) / 100.0f;  // 0.01 Hz => Hz
        break;
      case P108_QUERY_Wh_imp:
        return P108_data->modbus.read_32b_HoldingRegister(0x0A) * 10.0f;          // 0.01 kWh => Wh
        break;
      case P108_QUERY_Wh_exp:
        return P108_data->modbus.read_32b_HoldingRegister(0x08) * 10.0f;          // 0.01 kWh => Wh
        break;
      case P108_QUERY_Wh_tot:
        return P108_data->modbus.read_32b_HoldingRegister(0x00) * 10.0f;          // 0.01 kWh => Wh
        break;
    }
  }

  if (errorcode == 0) { return value; // DF - not present in P085
  }
  return 0.0f;
}

void p108_showValueLoadPage(uint8_t query, struct EventStruct *event) {
  addRowLabel(Plugin_108_valuename(query, true));
  addHtml(String(p108_readValue(query, event)));
}

#endif // ifdef USES_P108
