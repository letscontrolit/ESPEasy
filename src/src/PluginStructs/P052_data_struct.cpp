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
  constexpr size_t nrStrings = NR_ELEMENTS(strings);
  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

void P052_data_struct::setABCperiod(int hours)
{
  // Enable: write 1 ... 65534 to HR14 and bit1 set to 0 at HR19
  // Disable: write 0 or 65535 to HR14 and bit1 set to 1 at HR19


  // Read HR19
  uint8_t errorcode = 0;
  int     value     = modbus.readHoldingRegister(P052_HR19_METER_CONTROL, errorcode);

  // Clear bit 1 in register and write back HR19
  if (bitRead(value, 1)) {
    bitClear(value, 1);
    modbus.writeSingleRegister(P052_HR19_METER_CONTROL, value, errorcode);
  }

  // Read HR14 and verify desired ABC period
  value = modbus.readHoldingRegister(P052_HR14_ABC_PERIOD, errorcode);

  // If HR14 (ABC period) is not the desired period,
  // write desired ABC period to HR14
  if (value != hours) {
    modbus.writeSingleRegister(P052_HR14_ABC_PERIOD, hours, errorcode);
  }
}

uint32_t P052_data_struct::getSensorID()
{
  uint8_t errorcode       = 0;
  const uint32_t sensorId = (modbus.readInputRegister(P052_IR30_SENSOR_ID_HIGH, errorcode) << 16) |
                            modbus.readInputRegister(P052_IR31_SENSOR_ID_LOW, errorcode);

  if (errorcode == 0) {
    return sensorId;
  }
  return 0;
}

bool P052_data_struct::readInputRegister(short addr, int& value)
{
  uint8_t errorcode = 0;

  value = modbus.readInputRegister(addr, errorcode);
  return errorcode == 0;
}

bool P052_data_struct::readHoldingRegister(short addr, int& value)
{
  uint8_t errorcode = 0;

  value = modbus.readHoldingRegister(addr, errorcode);
  return errorcode == 0;
}

#endif // ifdef USES_P052
