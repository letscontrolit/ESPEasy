#include "_Plugin_Helper.h"

#ifdef USES_P199

// #######################################################################################################
// ############## Plugin 199: Modbus rain sensor                                           ###############
// #######################################################################################################

/*
   Plugin written by: Flashmark

   This plugin reads values from a Modbus RTU device.
 */


# define P199_DEBUG            
# define PLUGIN_199
# define PLUGIN_ID_199         199
# define PLUGIN_NAME_199       "[testing] Modbus RTU"
# define P199_NR_OUTPUT_VALUES 4
# define PLUGIN_VALUENAME1_199 "Value1"
# define PLUGIN_VALUENAME2_199 "Value2"
# define PLUGIN_VALUENAME3_199 "Value3"
# define PLUGIN_VALUENAME4_199 "Value4"

// Plugin configuration parameters
// PCONFIG(0) is the Modbus device ID, 
// PCONFIG(1) is the baud rate.
// PCONFIG(2) is used for flags, where bit 0 indicates collision detection
// PCONFIG(3) is Modbus register address for value 1
// PCONFIG(4) is Modbus register address for value 2
// PCONFIG(5) is Modbus register address for value 3
// PCONFIG(6) is Modbus register address for value 4
# define P199_DEV_ID           PCONFIG(0)
# define P199_DEV_ID_LABEL     PCONFIG_LABEL(0)
# define P199_BAUDRATE         PCONFIG(1)
# define P199_BAUDRATE_LABEL   PCONFIG_LABEL(1)
# define P199_ADDRESS(x)       PCONFIG(3 + x)
# define P199_ADDRESS_LABEL(x) concat(F("addr"), x) 

# define P199_GET_FLAG_COLL_DETECT bitRead(PCONFIG(2), 0)
# define P199_SET_FLAG_COLL_DETECT(x) bitWrite(PCONFIG(2), 0, x)
# define P199_FLAG_COLL_DETECT_LABEL "colldet"

# define P199_QUERY1_CONFIG_POS  3

# define P199_DEPIN           CONFIG_PIN3

# define P199_DEV_ID_DFLT     1
# define P199_BAUDRATE_DFLT   3 // 9600 baud

# include <ESPeasySerial.h>

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial *P199_ESPEasySerial = nullptr;
boolean P199_init                 = false;

void P199_scan_modbus();
void P199_scan_module(uint8_t node_id, uint8_t start_reg = 0x00, uint8_t end_reg = 0xFF);

boolean Plugin_199(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_199;
      dev.Type             = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      dev.VType            = Sensor_VType::SENSOR_TYPE_SINGLE; // Only one value
      dev.FormulaOption    = true;
      dev.ValueCount       = P199_NR_OUTPUT_VALUES;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_199);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_199));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_199));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_199));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_199));
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
      P199_DEV_ID   = P199_DEV_ID_DFLT;
      P199_BAUDRATE = P199_BAUDRATE_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((P199_DEV_ID == 0) || (P199_DEV_ID > 247) || (P199_BAUDRATE >= 6)) {
        // Load some defaults
        P199_DEV_ID   = P199_DEV_ID_DFLT;
        P199_BAUDRATE = P199_BAUDRATE_DFLT;
      }
      {
        String options_baudrate[6];

        for (int i = 0; i < 6; ++i) {
          options_baudrate[i] = P199_storageValueToBaudrate(i);
        }
        constexpr size_t optionCount = NR_ELEMENTS(options_baudrate);
        const FormSelectorOptions selector(optionCount, options_baudrate);
        selector.addFormSelector(F("Baud Rate"), P199_BAUDRATE_LABEL, P199_BAUDRATE);
        addUnit(F("baud"));
      }

      addFormNumericBox(F("Modbus Address"), P199_DEV_ID_LABEL, P199_DEV_ID, 1, 247);

      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), F(P199_FLAG_COLL_DETECT_LABEL), P199_GET_FLAG_COLL_DETECT);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      for (int outputIndex = 0; outputIndex < P199_NR_OUTPUT_VALUES; ++outputIndex)
      {
        addFormNumericBox(concat(F("Value "), outputIndex + 1), P199_ADDRESS_LABEL(outputIndex), P199_ADDRESS(outputIndex));
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

      P199_DEV_ID   = getFormItemInt(P199_DEV_ID_LABEL);
      P199_BAUDRATE = getFormItemInt(P199_BAUDRATE_LABEL);
      # ifdef ESP32
      P199_SET_FLAG_COLL_DETECT(isFormItemChecked(F(P199_FLAG_COLL_DETECT_LABEL)));
      # endif // ifdef ESP32
      
      for (int outputIndex = 0; outputIndex < P199_NR_OUTPUT_VALUES; ++outputIndex)
      {
        P199_ADDRESS(outputIndex) = getFormItemInt( P199_ADDRESS_LABEL(outputIndex));
      }

      P199_init = false; // Force device setup next time
      success         = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P199_init = true;

      // (re)create the serial port object
      // If the serial port object already exists, delete it first.
      if (P199_ESPEasySerial != nullptr) {
        delete P199_ESPEasySerial;
        P199_ESPEasySerial = nullptr;
      }
      P199_ESPEasySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2);

      if (P199_ESPEasySerial == nullptr) {
        break;
      }

      // Set RS485 mode if requested using selected pin for RTS
      bool rs485Mode = P199_ESPEasySerial->setRS485Mode(P199_DEPIN, P199_GET_FLAG_COLL_DETECT);
    
      unsigned int baudrate = P199_storageValueToBaudrate(P199_BAUDRATE);
      P199_ESPEasySerial->begin(baudrate);

      #ifdef P199_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("P199: Init serial: RX pin ");
        log += CONFIG_PIN1;
        log += F(", TX pin ");
        log += CONFIG_PIN2;
        log += F(", RS485 mode selected on pin ");
        log += P199_DEPIN;
        log += F(", baudrate ");
        log += P199_storageValueToBaudrate(P199_BAUDRATE); 
        log += F(", collision detection ");
        log += P199_GET_FLAG_COLL_DETECT ? F("enabled") : F("disabled");
        log += F(", RS485mode enabled: ");
        log += rs485Mode ? F("yes") : F("no");
        addLogMove(LOG_LEVEL_INFO, log);
      }
      #endif // ifdef P199_DEBUG

      success         = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P199_init = false;

      delete P199_ESPEasySerial;
      P199_ESPEasySerial = nullptr;

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {

      break;
    }
    
    case PLUGIN_READ:
    {
      uint16_t value = 0;
      for (int outputIndex = 0; outputIndex < P199_NR_OUTPUT_VALUES; ++outputIndex)
      {
        P199_modbus_readRegister(P199_DEV_ID, P199_ADDRESS(outputIndex), &value);
        UserVar.setFloat(event->TaskIndex, outputIndex, value);
      }
      break;
    }
    case PLUGIN_WRITE:
    {
      if (P199_ESPEasySerial != nullptr) {
        const String cmd = parseString(string, 1);

        if (equals(cmd, F("modbus"))) {
          const String subcmd = parseString(string, 2);

          if (equals(subcmd, F("write"))) {
            // Write a value to a Modbus register
            int address = parseString(string, 3).toInt();
            uint16_t value = parseString(string, 4).toInt();
            P199_modbus_writeRegister(P199_DEV_ID, address, value);
            String log = F("Modbus: write value ");
            log += value;
            log += F(" to address ");
            log += address;
            addLogMove(LOG_LEVEL_INFO, log);
            success = true;
          } 
          else if (equals(subcmd, F("scan"))) {
            // Scan for Modbus devices
            addLogMove(LOG_LEVEL_INFO, F("Modbus: Scanning for Modbus modules"));
            P199_scan_modbus();
            success = true;
          } 
          else if (equals(subcmd, F("read"))) {
            // Read a value from a Modbus register
            int address = parseString(string, 3).toInt();
            uint16_t value = 0;
            P199_modbus_readRegister(P199_DEV_ID, address, &value);
            String log = F("Modbus: read value ");
            log += value;
            log += F(" from address ");
            log += address;
            addLogMove(LOG_LEVEL_INFO, log);
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
            P199_scan_module(P199_DEV_ID, start_address, end_address);
            success = true;
          }
          else {
            addLogMove(LOG_LEVEL_ERROR, F("Modbus: Unknown command"));
          }
        }
      }
      break;
    }

  }
  return success;
}

int P199_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  if (baudrate_setting < 6) {
    baudrate = 1200 << baudrate_setting;
  }
  return baudrate;
}

int P199_modbus_readRegister(uint8_t node_id, uint16_t reg, uint16_t *value)
{
  uint8_t buffer[8];    // Buffer for Modbus request
  uint8_t response[8];  // Buffer for Modbus response

  buffer[0] = node_id;
  buffer[1] = 0x03; // Function code for reading holding registers
  buffer[2] = highByte(reg); // High byte of register address
  buffer[3] = lowByte(reg); // Low byte of register address
  buffer[4] = 0x00; // Number of registers to read (2 bytes)
  buffer[5] = 0x01; // Number of registers to read (2 bytes)
  uint16_t crc = P199_calculateCRC((uint8_t*)buffer, 6);
  buffer[6] = lowByte(crc); // CRC low byte
  buffer[7] = highByte(crc); // CRC high byte
  
  if (P199_modbus_exchange_message(buffer, response, 8, 7) < 0) {
    return -1; // Failed to exchange message
  }

  if (response[0] == node_id && response[1] == 0x03 && response[2] == 0x02) {
    *value = (response[3] << 8) | response[4]; // Combine high and low byte
    addLogMove(LOG_LEVEL_DEBUG, concat("Modbus: received value: ", *value));
    return 0; // Success
  } else {
    addLogMove(LOG_LEVEL_DEBUG, F("Modbus: Invalid response received"));      
    return -2; // Invalid response
  }
}

int P199_modbus_writeRegister(uint8_t node_id, uint16_t reg, uint16_t value)
{
  uint8_t buffer[8];
  uint8_t response[8];

  buffer[0] = node_id;
  buffer[1] = 0x06;            // Function code for reading holding registers
  buffer[2] = highByte(reg);   // High byte of register address
  buffer[3] = lowByte(reg);    // Low byte of register address
  buffer[4] = highByte(value); // High byte of value to write
  buffer[5] = lowByte(value);  // Low byte of value to write
  uint16_t crc = P199_calculateCRC((uint8_t*)buffer, 6);
  buffer[6] = lowByte(crc); // CRC low byte
  buffer[7] = highByte(crc); // CRC high byte

  if (P199_modbus_exchange_message(buffer, response, 8, 7) < 0) {
    return -1; // Failed to exchange message
  }
  if (response[0] == node_id && response[1] == 0x06 && response[2] == highByte(reg) && response[3] == lowByte(reg)) {
      uint16_t crc = P199_calculateCRC((uint8_t*)response, 6);
      if (response[5] != lowByte(crc) || response[6] != highByte(crc)) {
        addLogMove(LOG_LEVEL_DEBUG, F("Modbus: Invalid CRC in response"));      
        return -2; // Invalid response
      }
      addLogMove(LOG_LEVEL_DEBUG, concat("Modbus: Success send value  ", value));
      return 0; // Success
  } else {
    addLogMove(LOG_LEVEL_DEBUG, F("Modbus: Invalid response received"));      
    return -2; // Invalid response
  } 
}

// Exchange Modbus RTU messages. Send the tx_buffer and wait for a response in rx_buffer.
int P199_modbus_exchange_message(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t tx_size, uint8_t rx_size)
{

  if (P199_ESPEasySerial == nullptr) {
    addLogMove(LOG_LEVEL_DEBUG, F("Modbus: Serial not initialized"));
    return -1; // Not initialized
  }

  for (int i = P199_ESPEasySerial->available(); i > 0; --i) {
    P199_ESPEasySerial->read(); // Clear any existing data in the buffer
  }

  P199_dump_buffer((uint8_t*)tx_buffer, tx_size); // Debug: Dump the transmit buffer content
  P199_ESPEasySerial->write((uint8_t*)tx_buffer, tx_size);
  unsigned long startTime = millis();
  while (P199_ESPEasySerial->available() < rx_size && (millis() - startTime) < 1000) {
    delay(10); // Wait for response
  }
  if (P199_ESPEasySerial->available() >= rx_size) {
    
    P199_ESPEasySerial->readBytes(rx_buffer, rx_size);
    P199_dump_buffer((uint8_t*)rx_buffer, rx_size); // Debug: Dump the receive buffer content
    return 0;
  } else {
    addLogMove(LOG_LEVEL_DEBUG, F("Modbus: Timeout waiting for response"));
    return -3; // Timeout
  } 
}

// Calculate CRC-16 for Modbus RTU
// This function calculates the CRC-16 checksum for a given array of bytes.
uint16_t P199_calculateCRC(const uint8_t *array, uint8_t len)  {
  uint16_t _crc, _flag;
  _crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    _crc ^= (uint16_t)array[i];
    for (uint8_t j = 8; j; j--) {
      _flag = _crc & 0x0001;
      _crc >>= 1;
      if (_flag)
        _crc ^= 0xA001;
    }
  }
  return _crc;
}

// Dump the content of a buffer to the log
// This function takes a pointer to a buffer and its length, and logs the content in hexadecimal format.
void P199_dump_buffer(const uint8_t *buffer, size_t length) {
#ifdef P199_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Modbus: Dumping buffer: ");
    for (size_t i = 0; i < length; ++i) {
      log += String(buffer[i], HEX);
      if (i < length - 1) {
        log += F(", ");
      }
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifdef P199_DEBUG
}

// Scan Modbus registers from 0x00 to 0xFF for a given node ID
void P199_scan_module(uint8_t node_id, uint8_t start_reg, uint8_t end_reg)
{
   String log;
   uint16_t value = 0;
    for (uint8_t reg = start_reg; reg <= end_reg; reg++) {
      int result = P199_modbus_readRegister(node_id, reg, &value);
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
void P199_scan_modbus()
{
   String log;
   uint16_t value = 0;
    for (uint8_t id = 0; id <= 247; id++) {
      int result = P199_modbus_readRegister(id, 1, &value);
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
#endif // USES_P199
