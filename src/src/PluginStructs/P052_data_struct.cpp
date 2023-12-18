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
  const __FlashStringHelper *strings[] {
    F("Empty"),                  F(""),
    F("Carbon Dioxide"),         F("co2"),
    F("Temperature"),            F("T"),
    F("Humidity"),               F("H"),
    F("Relay Status"),           F("rel"),
    F("Temperature Adjustment"), F("Tadj"),
    F("ABC period"),             F("abc_per"),
    F("Error Status"),           F("err")
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
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

const char p052_subcommands[] PROGMEM = "writehr|readhr|enableabc|disableabc|setabcperiod|setrelay";
enum class p052_subcommands_e {
  writehr, // Modbus Write Holding Register
  readhr,  // Modbus Read Holding Register
  enableabc,
  disableabc,
  setabcperiod,
  setrelay
};


bool P052_data_struct::plugin_write(struct EventStruct *event, const String& string)
{
  if (!isInitialized()) {
    return false;
  }

  bool success         = true;
  const String command = parseString(string, 1);

  if (equals(command, F("senseair"))) {
    const String subcommand = parseString(string, 2);

    const int command_i = GetCommandCode(subcommand.c_str(), p052_subcommands);

    if (command_i == -1) {
      // No matching subcommand found
      return false;
    }


    switch (static_cast<p052_subcommands_e>(command_i)) {
      case p052_subcommands_e::writehr:
      {
        // FIXME TD-er: Must test, never tested on real hardware yet....

        uint32_t addr = 0;
        uint32_t cmnd = 0;

        if (validUIntFromString(parseString(string, 3), addr) &&
            validUIntFromString(parseString(string, 4), cmnd)) {
          uint8_t errorcode = 0;
          modbus.writeSingleRegister(addr, cmnd, errorcode);
          success = 0 == errorcode;

          if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, strformat(
                     F("Senseair command: %s=%d,%d"),
                     subcommand.c_str(),
                     addr,
                     cmnd));
          }
        }

        // Already sent log, thus return here
        return success;
      }
      case p052_subcommands_e::readhr:
      {
        uint32_t addr = 0;

        if (validUIntFromString(parseString(string, 3), addr)) {
          uint8_t   errorcode = 0;
          const int value     = modbus.readHoldingRegister(addr, errorcode);

          if (0 == errorcode) {
            success = true;

            if (Settings.UseRules) {
              eventQueue.add(
                event->TaskIndex,
                subcommand,
                strformat(
                  F("%d,%d"),
                  (addr + 1), // HR1 = addr 0x00
                  value));
            }
          }
        }
        break;
      }
      case p052_subcommands_e::enableabc:
      {
        uint32_t hours = 0;

        if (validUIntFromString(parseString(string, 3), hours)) {
          // FIXME TD-er: Implement


          // Read HR19

          // Clear bit 1 in register and write back HR19

          // Read HR14 and verify desired ABC period

          // If HR14 (ABC period) is not the desired period,
          // write desired ABC period to HR14
        }
        break;
      }
      case p052_subcommands_e::disableabc:
      {
        // FIXME TD-er: Implement

        // Read HR19

        // Set bit 1 in register and write back HR19
        break;
      }
      case p052_subcommands_e::setabcperiod:
      {
        // FIXME TD-er: Must test, never tested on real hardware yet....

        int32_t period = 0;

        if (validIntFromString(parseString(string, 3), period)) {
          if (period >= 0) {
            setABCperiod(period);
            success = true;
          }
        }
        break;
      }
      case p052_subcommands_e::setrelay:
      {
        int32_t state = 0;

        if (validIntFromString(parseString(string, 3), state)) {
          short relaystatus = 1;

          //  Refer to sensor model’s specification for voltage at 100% output.
          if (state == 0) { relaystatus = 0; }
          else if (state == 1) { relaystatus = 0x3FFF; } // 0x3FFF represents 100% output.
          else if (state == -1) { relaystatus = 0x7FFF; }

          if (1 != relaystatus) {
            modbus.writeSingleRegister(0x18, relaystatus);
            success = true;
          }
        }
        break;
      }
    }

    if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(
               F("Senseair command: %s=%d"),
               subcommand.c_str(),
               event->Par2));
    }
  }
  return success;
}

#endif // ifdef USES_P052
