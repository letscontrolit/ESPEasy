#include "_Plugin_Helper.h"

#ifdef USES_P183

// #######################################################################################################
// ############## Plugin 183: Modbus RTU generic sensor interface                          ###############
// #######################################################################################################

/*
   Plugin written by: Flashmark

   This plugin reads values from a Modbus RTU device.
 */

/**
 * Changelog:
 * 2025-08-24 flasmark: Initial version
 */

# define P183_DEBUG // Switch on additional debug logging
# define PLUGIN_183
# define PLUGIN_ID_183         183
# define PLUGIN_NAME_183       "[testing] Modbus RTU"
# define P183_NR_OUTPUT_VALUES 4
# define PLUGIN_VALUENAME1_183 "Value1"
# define PLUGIN_VALUENAME2_183 "Value2"
# define PLUGIN_VALUENAME3_183 "Value3"
# define PLUGIN_VALUENAME4_183 "Value4"

// Plugin configuration parameters
// PCONFIG(0) is the Modbus device ID.
// PCONFIG(1) is the serial baud rate.
// PCONFIG(2) is used for flags, where bit 0 indicates collision detection
// PCONFIG(3) is the number of active output values (1-4)
// PCONFIG(4) is the Modbus register address for value 1
// PCONFIG(5) is the Modbus register address for value 2
// PCONFIG(6) is the Modbus register address for value 3
// PCONFIG(7) is the Modbus register address for value 4
// Use P183_ADDRESS(x) to access the PCONFIG value for value x
# define P183_DEV_ID           PCONFIG(0)
# define P183_DEV_ID_LABEL     PCONFIG_LABEL(0)
# define P183_BAUDRATE         PCONFIG(1)
# define P183_BAUDRATE_LABEL   PCONFIG_LABEL(1)
# define P183_NR_OUTPUTS       PCONFIG(3)
# define P183_NR_OUTPUTS_LABEL PCONFIG_LABEL(3)
# define P183_ADDRESS(x) PCONFIG(4 + x)
# define P183_ADDRESS_LABEL(x) concat(F("addr"), x)

# define P183_GET_FLAG_COLL_DETECT bitRead(PCONFIG(2), 0)
# define P183_SET_FLAG_COLL_DETECT(x) bitWrite(PCONFIG(2), 0, x)
# define P183_FLAG_COLL_DETECT_LABEL "colldet"

# define P183_DEPIN           CONFIG_PIN3

# define P183_DEV_ID_DFLT     1
# define P183_BAUDRATE_DFLT   3 // 9600 baud

# define P183_MAX_BAUDRATE_SEL  8

# include <ESPeasySerial.h>
# include "src/Helpers/Modbus_device.h"
# include "src/Helpers/Modbus_mgr.h"

// Modbus properties
# define P183_MAX_MODBUS_NODES 247
# define P183_MODBUS_TIMEOUT   1000 // milliseconds
# define P183_MODBUS_BROADCAST_ID 0 // Modbus broadcast address
# define P183_MODBUS_FUNC_READ_HOLDING_REGISTERS 0x03
# define P183_MODBUS_FUNC_WRITE_SINGLE_REGISTER  0x06

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ModbusDEVICE_struct * P183_ModbusDevice = nullptr;
ModbusQueueState_t P183_ModbusStatus =   ModbusQueueState_t::EMPTY;
boolean P183_init                    = false;

void P183_scan_modbus();
void P183_scan_module(uint8_t node_id,
                      uint8_t start_reg = 0x00,
                      uint8_t end_reg   = 0xFF);

boolean Plugin_183(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_183;
      dev.Type             = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      dev.VType            = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption    = true;
      dev.ValueCount       = P183_NR_OUTPUT_VALUES;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_183);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_183));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_modbus_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P183_DEV_ID   = P183_DEV_ID_DFLT;
      P183_BAUDRATE = P183_BAUDRATE_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((P183_DEV_ID <= 0) || (P183_DEV_ID > P183_MAX_MODBUS_NODES) || (P183_BAUDRATE >= 6)) {
        // Load some defaults
        P183_DEV_ID   = P183_DEV_ID_DFLT;
        P183_BAUDRATE = P183_BAUDRATE_DFLT;
      }
      {
        String options_baudrate[P183_MAX_BAUDRATE_SEL];

        for (int i = 0; i < P183_MAX_BAUDRATE_SEL; ++i) {
          options_baudrate[i] = P183_storageValueToBaudrate(i);
        }
        constexpr size_t optionCount = NR_ELEMENTS(options_baudrate);
        const FormSelectorOptions selector(optionCount, options_baudrate);
        selector.addFormSelector(F("Baud Rate"), P183_BAUDRATE_LABEL, P183_BAUDRATE);
        addUnit(F("baud"));
      }

      addFormNumericBox(F("Modbus Device Address"), P183_DEV_ID_LABEL, P183_DEV_ID, 1, 247);

      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), F(P183_FLAG_COLL_DETECT_LABEL), P183_GET_FLAG_COLL_DETECT);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      if ((P183_NR_OUTPUTS < 1) || (P183_NR_OUTPUTS > P183_NR_OUTPUT_VALUES)) {
        P183_NR_OUTPUTS = P183_NR_OUTPUT_VALUES; // Default to max outputs
      }
      addFormNumericBox(F("Number of values to read"), P183_NR_OUTPUTS_LABEL, P183_NR_OUTPUTS);

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUT_VALUES; ++outputIndex)
      {
        addFormNumericBox(concat(F("Holding Register for value"), outputIndex + 1), P183_ADDRESS_LABEL(outputIndex),
                          P183_ADDRESS(outputIndex));
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P183_DEV_ID   = getFormItemInt(P183_DEV_ID_LABEL);
      P183_BAUDRATE = getFormItemInt(P183_BAUDRATE_LABEL);
      # ifdef ESP32
      P183_SET_FLAG_COLL_DETECT(isFormItemChecked(F(P183_FLAG_COLL_DETECT_LABEL)));
      # endif // ifdef ESP32

      P183_NR_OUTPUTS = getFormItemInt(P183_NR_OUTPUTS_LABEL);

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUT_VALUES; ++outputIndex)
      {
        P183_ADDRESS(outputIndex) = getFormItemInt(P183_ADDRESS_LABEL(outputIndex));
      }

      P183_init = false; // Force device setup next time
      success   = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P183_init = true;

      // (re)create the serial port object
      // If the serial port object already exists, delete it first.
      if (P183_ModbusDevice != nullptr) {
        delete P183_ModbusDevice;
        P183_ModbusDevice = nullptr;
      }
      P183_ModbusDevice = new ModbusDEVICE_struct();

      if (P183_ModbusDevice == nullptr) {
        P183_init = false;
        addLogMove(LOG_LEVEL_ERROR, F("P183: Unable to allocate Modbus device object"));
        break;
      }

      if (!P183_ModbusDevice->init(P183_DEV_ID, static_cast<ESPEasySerialPort>(CONFIG_PORT),
                                   CONFIG_PIN1,
                                   CONFIG_PIN2,
                                   P183_storageValueToBaudrate(P183_BAUDRATE),
                                   P183_DEPIN,
                                   P183_GET_FLAG_COLL_DETECT)) {
        break;
      }
      P183_ModbusDevice->setModbusTimeout(P183_MODBUS_TIMEOUT);

      # ifdef P183_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("P183: Init serial: RX pin ");
        log += CONFIG_PIN1;
        log += F(", TX pin ");
        log += CONFIG_PIN2;
        log += F(", RS485 mode selected on pin ");
        log += P183_DEPIN;
        log += F(", baudrate ");
        log += P183_storageValueToBaudrate(P183_BAUDRATE);
        log += F(", collision detection ");
        log += P183_GET_FLAG_COLL_DETECT ? F("enabled") : F("disabled");
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifdef P183_DEBUG

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P183_init = false;
      delete P183_ModbusDevice;
      P183_ModbusDevice = nullptr;
      success           = true;
      break;
    }

    case PLUGIN_READ:
    {
      uint16_t value = 0;

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUTS; ++outputIndex)
      {
        P183_modbus_readRegister(P183_ADDRESS(outputIndex), &value);
        UserVar.setFloat(event->TaskIndex, outputIndex, value);
      }
      success = true;
      break;
    }
    case PLUGIN_WRITE:
    {
      if (P183_ModbusDevice != nullptr) {
        const String cmd = parseString(string, 1);

        if (equals(cmd, F("modbus"))) {
          const String subcmd = parseString(string, 2);

          if (equals(subcmd, F("write"))) {
            // Write a value to a Modbus register
            int address    = parseString(string, 3).toInt();
            uint16_t value = parseString(string, 4).toInt();
            P183_modbus_writeRegister(P183_DEV_ID, address, value);

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("Modbus: write value ");
              log += value;
              log += F(" to address ");
              log += address;
              addLogMove(LOG_LEVEL_INFO, log);
            }
            success = true;
          }
          else if (equals(subcmd, F("read"))) {
            // Read a value from a Modbus register
            int address    = parseString(string, 3).toInt();
            uint16_t value = 0;
            P183_modbus_readRegister(address, &value);

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("Modbus: read value ");
              log += value;
              log += F(" from address ");
              log += address;
              addLogMove(LOG_LEVEL_INFO, log);
            }
            success = true;
          }
          else if (equals(subcmd, F("dump"))) {
            int start_address = parseString(string, 3).toInt();
            int end_address   = parseString(string, 4).toInt();

            if (end_address < start_address) {
              end_address = start_address;
            }

            if (end_address - start_address > 100) {
              end_address = start_address + 100; // Limit to 100 addresses
            }
            addLogMove(LOG_LEVEL_INFO, F("Modbus: dumping module registers"));
            P183_scan_module(P183_DEV_ID, start_address, end_address);
            success = true;
          }
          else if (equals(subcmd, F("scan"))) {
            // Scan for Modbus devices
            addLogMove(LOG_LEVEL_INFO, F("Modbus: Scanning for Modbus modules"));
            P183_scan_modbus();
            success = true;
          }
          else {
            addLogMove(LOG_LEVEL_ERROR, F("Modbus: Unknown command"));
          }
        }
      }
      break;
    }
    case PLUGIN_GET_CONFIG_VALUE: {
      const String cmd = parseString(string, 1);

      if (equals(cmd, F("register"))) {
        int address    = parseString(string, 2).toInt();
        uint16_t value = 0;
        P183_modbus_readRegister(address, &value);
        string  = String(value);
        success = true;
      }
      break;
    }
  }
  return success;
}

// Convert stored baudrate setting (enumeration value) to actual baudrate value
// Returns the actual baudrate value.
int P183_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  switch (baudrate_setting)
  {
    case 0:
      baudrate = 1200;   break;
    case 1:
      baudrate = 2400;   break;
    case 2:
      baudrate = 4800;   break;
    case 3:
      baudrate = 9600;   break;
    case 4:
      baudrate = 19200;  break;
    case 5:
      baudrate = 38400;  break;
    case 6:
      baudrate = 57600;  break;
    case 7:
      baudrate = 115200; break;
    default:
      baudrate = 9600;   break; // Default value for fallback
  }
  return baudrate;
}

// Read a single Modbus register from a device with given node ID
// On success, the read value is stored in *value and 0 is returned.
int P183_modbus_readRegister(uint16_t reg, uint16_t *value)
{
  if (P183_ModbusDevice != nullptr) {
    P183_ModbusDevice->readHoldingRegister(reg, value, &P183_ModbusStatus);
  }
  return 0;
}

// Write a single Modbus register to a device with given node ID
// On success, 0 is returned.
int P183_modbus_writeRegister(uint8_t node_id, uint16_t reg, uint16_t value)
{
  if (P183_ModbusDevice != nullptr) {
    P183_ModbusDevice->writeSingleRegister(reg, value, &P183_ModbusStatus);
  }
  return 0;
}

// Dump the content of a buffer to the log
// This function takes a pointer to a buffer and its length, and logs the content in hexadecimal format.
void P183_dump_buffer(const uint8_t *buffer, size_t length) {
# ifdef P183_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("Modbus: Dumping buffer: ");

    for (size_t i = 0; i < length; ++i) {
      log += String(buffer[i], HEX);

      if (i < length - 1) {
        log += F(", ");
      }
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
# endif // ifdef P183_DEBUG
}

// Scan Modbus registers from 0x00 to 0xFF for a given node ID
void P183_scan_module(uint8_t node_id, uint8_t start_reg, uint8_t end_reg)
{
  String   log;
  uint16_t value = 0;

  for (uint8_t reg = start_reg; reg <= end_reg; reg++) {
    int result = P183_modbus_readRegister(reg, &value);
    log += F("** Address ");
    log += String(reg);
    log += F(" (0x");
    log += String(reg, HEX);

    if (result == 0) {
      log += F(") = ");
      log += String(value);
    } else {
      log += F(") invalid");
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

// Scan Modbus addreses from 0x00 to 0xFF for a given node ID
void P183_scan_modbus()
{
  String   log;
  uint16_t value = 0;

  for (uint8_t id = 0; id <= 247; id++) {
    //TODO: how to scan the Modbus devices in teh new structure
    int result = P183_modbus_readRegister(1, &value);
    log += F("** Address ");
    log += String(id);

    if (result == 0) {
      log += F(" OK");
    } else {
      log += F(" no response");
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

#endif // USES_P183
