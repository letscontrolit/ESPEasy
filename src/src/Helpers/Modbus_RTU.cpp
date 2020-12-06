#include "Modbus_RTU.h"


#include "../ESPEasyCore/ESPEasy_Log.h"
#include "ESPEasy_time_calc.h"
#include "StringConverter.h"


ModbusRTU_struct::ModbusRTU_struct() : easySerial(nullptr) {
  reset();
}

ModbusRTU_struct::~ModbusRTU_struct() {
  reset();
}

void ModbusRTU_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
  detected_device_description = "";

  for (int i = 0; i < 8; ++i) {
    _sendframe[i] = 0;
  }
  _sendframe_used = 0;

  for (int i = 0; i < MODBUS_RECEIVE_BUFFER; ++i) {
    _recv_buf[i] = 0xff;
  }
  _recv_buf_used    = 0;
  _modbus_address   = MODBUS_BROADCAST_ADDRESS;
  _reads_pass       = 0;
  _reads_crc_failed = 0;
  _reads_nodata     = 0;
}

bool ModbusRTU_struct::init(const ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, int16_t baudrate, byte address) {
  return init(port, serial_rx, serial_tx, baudrate, address, -1);
}

bool ModbusRTU_struct::init(const ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, int16_t baudrate, byte address, int8_t dere_pin) {
  if ((serial_rx < 0) || (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);
  if (easySerial == nullptr) { return false; }
  easySerial->begin(baudrate);

  if (!isInitialized()) { return false; }
  _modbus_address = address;
  _dere_pin       = dere_pin;

  if (_dere_pin != -1) { // set output pin mode for DE/RE pin when used (for control MAX485)
    pinMode(_dere_pin, OUTPUT);
  }

  detected_device_description = getDevice_description(_modbus_address);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log; // = F("Modbus detected: ");
    log += detected_device_description;
    addLog(LOG_LEVEL_INFO, log);
    modbus_log_MEI(_modbus_address);
  }
  return true;
}

bool ModbusRTU_struct::isInitialized() const {
  return easySerial != nullptr;
}

void ModbusRTU_struct::getStatistics(uint32_t& pass, uint32_t& fail, uint32_t& nodata) const {
  pass   = _reads_pass;
  fail   = _reads_crc_failed;
  nodata = _reads_nodata;
}

void ModbusRTU_struct::setModbusTimeout(uint16_t timeout) {
  _modbus_timeout = timeout;
}

uint16_t ModbusRTU_struct::getModbusTimeout() const {
  return _modbus_timeout;
}

String ModbusRTU_struct::getDevice_description(byte slaveAddress) {
  bool more_follows     = true;
  byte next_object_id   = 0;
  byte conformity_level = 0;
  unsigned int object_value_int;
  String description;
  String obj_text;

  for (byte object_id = 0; object_id < 0x84; ++object_id) {
    if (object_id == 6) {
      object_id = 0x82; // Skip to the serialnr/sensor type
    }
    int result = modbus_get_MEI(slaveAddress, object_id, obj_text,
                                object_value_int, next_object_id,
                                more_follows, conformity_level);
    String label;

    switch (object_id) {
      case 0x01:

        if (result == 0) { label = F("Pcode"); }
        break;
      case 0x02:

        if (result == 0) { label = F("Rev"); }
        break;
      case 0x82:
      {
        if (result != 0) {
          uint32_t sensorId = readSensorId();
          obj_text = String(sensorId, HEX);
          result   = 0;
        }

        label = F("S/N");
        break;
      }
      case 0x83:
      {
        if (result != 0) {
          uint32_t sensorId = readTypeId();
          obj_text = String(sensorId, HEX);
          result   = 0;
        }

        label = F("Type");
        break;
      }
      default:
        break;
    }

    if (result == 0) {
      if (label.length() > 0) {
        // description += MEI_objectid_to_name(object_id);
        description += label;
        description += ": ";
      }

      if (obj_text.length() > 0) {
        description += obj_text;
        description += " - ";
      }
    }
  }
  return description;
}

// Read from RAM or EEPROM
void ModbusRTU_struct::buildRead_RAM_EEPROM(byte slaveAddress, byte functionCode,
                                            short startAddress, byte number_bytes) {
  _sendframe[0]   = slaveAddress;
  _sendframe[1]   = functionCode;
  _sendframe[2]   = (byte)(startAddress >> 8);
  _sendframe[3]   = (byte)(startAddress & 0xFF);
  _sendframe[4]   = number_bytes;
  _sendframe_used = 5;
}

// Write to the Special Control Register (SCR)
void ModbusRTU_struct::buildWriteCommandRegister(byte slaveAddress, byte value) {
  _sendframe[0]   = slaveAddress;
  _sendframe[1]   = MODBUS_CMD_WRITE_RAM;
  _sendframe[2]   = 0;    // Address-Hi SCR  (0x0060)
  _sendframe[3]   = 0x60; // Address-Lo SCR
  _sendframe[4]   = 1;    // Count
  _sendframe[5]   = value;
  _sendframe_used = 6;
}

void ModbusRTU_struct::buildWriteMult16bRegister(byte slaveAddress, uint16_t startAddress, uint16_t value) {
  _sendframe[0]   = slaveAddress;
  _sendframe[1]   = MODBUS_WRITE_MULTIPLE_REGISTERS;
  _sendframe[2]   = (byte)(startAddress >> 8);
  _sendframe[3]   = (byte)(startAddress & 0xFF);
  _sendframe[4]   = 0; // nr reg hi
  _sendframe[5]   = 1; // nr reg lo
  _sendframe[6]   = 2; // nr bytes to follow (2 bytes per register)
  _sendframe[7]   = (byte)(value >> 8);
  _sendframe[8]   = (byte)(value & 0xFF);
  _sendframe_used = 9;
}

void ModbusRTU_struct::buildFrame(byte slaveAddress, byte functionCode,
                                  short startAddress, short parameter) {
  _sendframe[0]   = slaveAddress;
  _sendframe[1]   = functionCode;
  _sendframe[2]   = (byte)(startAddress >> 8);
  _sendframe[3]   = (byte)(startAddress & 0xFF);
  _sendframe[4]   = (byte)(parameter >> 8);
  _sendframe[5]   = (byte)(parameter & 0xFF);
  _sendframe_used = 6;
}

void ModbusRTU_struct::build_modbus_MEI_frame(byte slaveAddress, byte device_id,
                                              byte object_id) {
  _sendframe[0] = slaveAddress;
  _sendframe[1] = 0x2B;
  _sendframe[2] = 0x0E;

  // The parameter "Read Device ID code" allows to define four access types :
  // 01: request to get the basic device identification (stream access)
  // 02: request to get the regular device identification (stream access)
  // 03: request to get the extended device identification (stream access)
  // 04: request to get one specific identification object (individual access)
  _sendframe[3]   = device_id;
  _sendframe[4]   = object_id;
  _sendframe_used = 5;
}

String ModbusRTU_struct::MEI_objectid_to_name(byte object_id) {
  String result;

  switch (object_id) {
    case 0:    result = F("VendorName");          break;
    case 1:    result = F("ProductCode");         break;
    case 2:    result = F("MajorMinorRevision");  break;
    case 3:    result = F("VendorUrl");           break;
    case 4:    result = F("ProductName");         break;
    case 5:    result = F("ModelName");           break;
    case 6:    result = F("UserApplicationName"); break;
    case 0x80: result = F("MemoryMapVersion");    break;
    case 0x81: result = F("Firmware Rev.");       break;
    case 0x82: result = F("Sensor S/N");          break;
    case 0x83: result = F("Sensor type");         break;
    default:
      result = formatToHex(object_id);
      break;
  }
  return result;
}

String ModbusRTU_struct::parse_modbus_MEI_response(unsigned int& object_value_int,
                                                   byte        & next_object_id,
                                                   bool        & more_follows,
                                                   byte        & conformity_level) {
  String result;

  if (_recv_buf_used < 8) {
    // Too small.
    addLog(LOG_LEVEL_INFO,
           String(F("MEI response too small: ")) + _recv_buf_used);
    next_object_id = 0xFF;
    more_follows   = false;
    return result;
  }
  int pos = 4; // Data skipped: slave_address, FunctionCode, MEI type, ReadDevId
  // See http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  // Page 45
  conformity_level = _recv_buf[pos++];
  more_follows     = _recv_buf[pos++] != 0;
  next_object_id   = _recv_buf[pos++];
  const byte number_objects = _recv_buf[pos++];
  byte object_id            = 0;

  for (int i = 0; i < number_objects; ++i) {
    if ((pos + 3) < _recv_buf_used) {
      object_id = _recv_buf[pos++];
      const byte object_length = _recv_buf[pos++];

      if ((pos + object_length) < _recv_buf_used) {
        String object_value;

        if (object_id < 0x80) {
          // Parse as type String
          object_value.reserve(object_length);
          object_value_int = static_cast<unsigned int>(-1);

          for (int c = 0; c < object_length; ++c) {
            object_value += char(_recv_buf[pos++]);
          }
        } else {
          object_value.reserve(2 * object_length + 2);
          object_value_int = 0;

          for (int c = 0; c < object_length; ++c) {
            object_value_int =
              object_value_int << 8 | _recv_buf[pos++];
          }
          object_value += formatToHex(object_value_int);
        }

        if (i != 0) {
          // Append to existing description
          result += ", ";
        }
        result += object_value;
      }
    }
  }
  return result;
}

void ModbusRTU_struct::logModbusException(byte value) {
  if (value == 0) {
    return;
  }

  /*
     // Exception Response, see:
     // http://digital.ni.com/public.nsf/allkb/E40CA0CFA0029B2286256A9900758E06?OpenDocument
     String log = F("Modbus Exception - ");
     switch (value) {
     case MODBUS_EXCEPTION_ILLEGAL_FUNCTION: {
      // The function code received in the query is not an allowable action for
      // the slave.
      // If a Poll Program Complete command was issued, this code indicates that
      // no program function preceded it.
      log += F("Illegal Function (not allowed by client)");
      break;
     }
     case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS: {
      // The data address received in the query is not an allowable address for
      // the slave.
      log += F("Illegal Data Address");
      break;
     }
     case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE: {
      // A value contained in the query data field is not an allowable value for
      // the slave
      log += F("Illegal Data Value");
      break;
     }
     case MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE: {
      // An unrecoverable error occurred while the slave was attempting to perform
      // the requested action
      log += F("Slave Device Failure");
      break;
     }
     case MODBUS_EXCEPTION_ACKNOWLEDGE: {
      // The slave has accepted the request and is processing it, but a long
      // duration of time will be
      // required to do so. This response is returned to prevent a timeout error
      // from occurring in the master.
      // The master can next issue a Poll Program Complete message to determine if
      // processing is completed.
      log += F("Acknowledge");
      break; // Is this an error?
     }
     case MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY: {
      // The slave is engaged in processing a long-duration program command.
      // The master should retransmit the message later when the slave is free.
      log += F("Slave Device Busy");
      break;
     }
     case MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE:
      log += F("Negative acknowledge");
      break;
     case MODBUS_EXCEPTION_MEMORY_PARITY:
      log += F("Memory parity error");
      break;
     case MODBUS_EXCEPTION_GATEWAY_PATH:
      log += F("Gateway path unavailable");
      break;
     case MODBUS_EXCEPTION_GATEWAY_TARGET:
      log += F("Target device failed to respond");
      break;
     case MODBUS_BADCRC:
      log += F("Invalid CRC");
      break;
     case MODBUS_BADDATA:
      log += F("Invalid data");
      break;
     case MODBUS_BADEXC:
      log += F("Invalid exception code");
      break;
     case MODBUS_MDATA:
      log += F("Too many data");
      break;
     case MODBUS_BADSLAVE:
      log += F("Response not from requested slave");
      break;
     case MODBUS_TIMEOUT:
      log += F("Modbus Timeout");
      break;
     case MODBUS_NODATA:
      log += F("Modbus No Data");
      break;
     default:
      log += String(F("Unknown Exception code: ")) + value;
      break;
     }
     log += F(" - sent: ");
     log += log_buffer(_sendframe, _sendframe_used);
     log += F(" - received: ");
     log += log_buffer(_recv_buf, _recv_buf_used);
     addLog(LOG_LEVEL_DEBUG_MORE, log);
   */
}

/*
   String log_buffer(byte *buffer, int length) {
    String log;
    log.reserve(3 * length + 5);
    for (int i = 0; i < length; ++i) {
      String hexvalue(buffer[i], HEX);
      hexvalue.toUpperCase();
      log += hexvalue;
      log += F(" ");
    }
    log += F("(");
    log += length;
    log += F(")");
    return log;
   }
 */
byte ModbusRTU_struct::processCommand() {
  // CRC-calculation
  unsigned int crc =
    ModRTU_CRC(_sendframe, _sendframe_used);

  // Note, this number has low and high bytes swapped, so use it accordingly (or
  // swap bytes)
  byte checksumHi = (byte)((crc >> 8) & 0xFF);
  byte checksumLo = (byte)(crc & 0xFF);

  _sendframe[_sendframe_used++] = checksumLo;
  _sendframe[_sendframe_used++] = checksumHi;

  int  nrRetriesLeft = 2;
  byte return_value  = 0;

  while (nrRetriesLeft > 0) {
    return_value = 0;

    // Send the byte array
    startWrite();
    easySerial->write(_sendframe, _sendframe_used);

    // sent all data from buffer
    easySerial->flush();
    startRead();

    // Read answer from sensor
    _recv_buf_used = 0;
    unsigned long timeout    = millis() + _modbus_timeout;
    bool validPacket         = false;
    bool invalidDueToTimeout = false;

    //  idx:    0,   1,   2,   3,   4,   5,   6,   7
    // send: 0x02,0x03,0x00,0x00,0x00,0x01,0x39,0x84
    // recv: 0x02,0x03,0x02,0x01,0x57,0xBC,0x2A

    while (!validPacket && !invalidDueToTimeout && _recv_buf_used < MODBUS_RECEIVE_BUFFER) {
      if (timeOutReached(timeout)) {
        invalidDueToTimeout = true;
      }

      while (!invalidDueToTimeout && easySerial->available() && _recv_buf_used < MODBUS_RECEIVE_BUFFER) {
        if (timeOutReached(timeout)) {
          invalidDueToTimeout = true;
        }
        _recv_buf[_recv_buf_used++] = easySerial->read();
      }

      if (_recv_buf_used > 2) {                                         // got length
        if (_recv_buf_used >= (3 + _recv_buf[2] + 2)) {                 // got whole pkt
          crc          = ModRTU_CRC(_recv_buf, _recv_buf_used);         // crc16 is 0 for whole valid pkt
          validPacket  = (crc == 0) && (_recv_buf[0] == _sendframe[0]); // check crc and address
          return_value = 0;                                             // reset return value
        }
      }
      delay(0);
    }

    // Check for MODBUS exception
    if (invalidDueToTimeout) {
      ++_reads_nodata;

      if (_recv_buf_used == 0) {
        return_value = MODBUS_NODATA;
      } else {
        return_value = MODBUS_TIMEOUT;
      }
    } else if (!validPacket) {
      ++_reads_crc_failed;
      return_value = MODBUS_BADCRC;
    } else {
      const byte received_functionCode = _recv_buf[1];

      if ((received_functionCode & 0x80) != 0) {
        return_value = _recv_buf[2];
      }
      ++_reads_pass;
      _reads_nodata = 0;
    }

    switch (return_value) {
      case MODBUS_EXCEPTION_ACKNOWLEDGE:
      case MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY:
      case MODBUS_BADCRC:
      case MODBUS_TIMEOUT:

        // Bad communication, makes sense to retry.
        break;
      default:
        nrRetriesLeft = 0; // When not supported, does not make sense to retry.
        break;
    }
    --nrRetriesLeft;
  }
  _last_error = return_value;
  return return_value;
}

uint32_t ModbusRTU_struct::read_32b_InputRegister(short address) {
  uint32_t result = 0;
  byte     errorcode;
  int idHigh = readInputRegister(address, errorcode);

  if (errorcode != 0) { return result; }
  int idLow = readInputRegister(address + 1, errorcode);

  if (errorcode == 0) {
    result  = idHigh;
    result  = result << 16;
    result += idLow;
  }
  return result;
}

uint32_t ModbusRTU_struct::read_32b_HoldingRegister(short address) {
  uint32_t result = 0;

  process_32b_register(_modbus_address, MODBUS_READ_HOLDING_REGISTERS, address, result);
  return result;
}

float ModbusRTU_struct::read_float_HoldingRegister(short address) {
  union {
    uint32_t ival;
    float    fval;
  } conversion;

  conversion.ival = read_32b_HoldingRegister(address);
  return conversion.fval;

  //    uint32_t ival = read_32b_HoldingRegister(address);
  //    float fval = *reinterpret_cast<float*>(&ival);
  //    return fval;
}

int ModbusRTU_struct::readInputRegister(short address, byte& errorcode) {
  // Only read 1 register
  return process_16b_register(_modbus_address, MODBUS_READ_INPUT_REGISTERS, address, 1, errorcode);
}

int ModbusRTU_struct::readHoldingRegister(short address, byte& errorcode) {
  // Only read 1 register
  return process_16b_register(
    _modbus_address, MODBUS_READ_HOLDING_REGISTERS, address, 1, errorcode);
}

// Write to holding register.
int ModbusRTU_struct::writeSingleRegister(short address, short value) {
  // No check for the specific error code.
  byte errorcode = 0;

  return writeSingleRegister(address, value, errorcode);
}

int ModbusRTU_struct::writeSingleRegister(short address, short value, byte& errorcode) {
  // GN: Untested, will probably not work
  return process_16b_register(
    _modbus_address, MODBUS_WRITE_SINGLE_REGISTER, address, value, errorcode);
}

// Function 16 (0x10) "Write Multiple Registers" to write to a single holding register
int ModbusRTU_struct::writeMultipleRegisters(short address, short value) {
  return preset_mult16b_register(
    _modbus_address, address, value);
}

byte ModbusRTU_struct::modbus_get_MEI(byte slaveAddress, byte object_id,
                                      String& result, unsigned int& object_value_int,
                                      byte& next_object_id, bool& more_follows,
                                      byte& conformity_level) {
  // Force device_id to 4 = individual access (reading one ID object per call)
  build_modbus_MEI_frame(slaveAddress, 4, object_id);
  const byte process_result = processCommand();

  if (process_result == 0) {
    result = parse_modbus_MEI_response(object_value_int,
                                       next_object_id, more_follows,
                                       conformity_level);
  } else {
    more_follows = false;
  }
  return process_result;
}

void ModbusRTU_struct::modbus_log_MEI(byte slaveAddress) {
  // Iterate over all Device identification items, using
  // Modbus command (0x2B / 0x0E) Read Device Identification
  // And add to log.
  bool more_follows     = true;
  byte conformity_level = 0;
  byte object_id        = 0;
  byte next_object_id   = 0;

  while (more_follows) {
    String result;
    unsigned int object_value_int;
    const byte   process_result = modbus_get_MEI(
      slaveAddress, object_id, result, object_value_int, next_object_id,
      more_follows, conformity_level);

    if (process_result == 0) {
      if (result.length() > 0) {
        String log = MEI_objectid_to_name(object_id);
        log += F(": ");
        log += result;
        addLog(LOG_LEVEL_INFO, log);
      }
    } else {
      switch (process_result) {
        case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:

          // No need to log this exception when scanning.
          break;
        default:
          logModbusException(process_result);
          break;
      }
    }

    // If more parts are needed, collect them or iterate over the known list.
    // For example with "individual access" a new request has to be sent for each single item
    if (more_follows) {
      object_id = next_object_id;
    } else if (object_id < 0x84) {
      // Allow for scanning only the usual object ID's
      // This range is vendor specific
      more_follows = true;
      object_id++;

      if (object_id == 7) {
        // Skip range 0x07...0x7F
        object_id = 0x80;
      }
    }
  }
}

int ModbusRTU_struct::process_16b_register(byte slaveAddress, byte functionCode,
                                           short startAddress, short parameter,
                                           byte& errorcode) {
  buildFrame(slaveAddress, functionCode, startAddress, parameter);
  errorcode = processCommand();

  if (errorcode == 0) {
    return (_recv_buf[3] << 8) | (_recv_buf[4]);
  }
  logModbusException(errorcode);
  return -1;
}

// Still writing single register, but calling it using "Preset Multiple Registers" function (FC=16)
int ModbusRTU_struct::preset_mult16b_register(byte slaveAddress, uint16_t startAddress, uint16_t value) {
  buildWriteMult16bRegister(slaveAddress, startAddress, value);
  const byte process_result = processCommand();

  if (process_result == 0) {
    return (_recv_buf[4] << 8) | (_recv_buf[5]);
  }
  logModbusException(process_result);
  return -1 * process_result;
}

bool ModbusRTU_struct::process_32b_register(byte slaveAddress, byte functionCode,
                                            short startAddress, uint32_t& result) {
  buildFrame(slaveAddress, functionCode, startAddress, 2);
  const byte process_result = processCommand();

  if (process_result == 0) {
    result = 0;

    for (byte i = 0; i < 4; ++i) {
      result  = result << 8;
      result += _recv_buf[i + 3];
    }
    return true;
  }
  logModbusException(process_result);
  return false;
}

int ModbusRTU_struct::writeSpecialCommandRegister(byte command) {
  buildWriteCommandRegister(_modbus_address, command);
  const byte process_result = processCommand();

  if (process_result == 0) {
    return 0;
  }
  logModbusException(process_result);
  return -1 * process_result;
}

unsigned int ModbusRTU_struct::read_RAM_EEPROM(byte command, byte startAddress,
                                               byte nrBytes,
                                               byte& errorcode) {
  buildRead_RAM_EEPROM(_modbus_address, command,
                       startAddress, nrBytes);
  errorcode = processCommand();

  if (errorcode == 0) {
    unsigned int result = 0;

    for (int i = 0; i < _recv_buf[2]; ++i) {
      // Most significant byte at lower address
      result = (result << 8) | _recv_buf[i + 3];
    }
    return result;
  }
  logModbusException(errorcode);
  return 0;
}

// Compute the MODBUS RTU CRC
unsigned int ModbusRTU_struct::ModRTU_CRC(byte *buf, int len) {
  unsigned int crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) { // Loop over each bit
      if ((crc & 0x0001) != 0) {   // If the LSB is set
        crc >>= 1;                 // Shift right and XOR 0xA001
        crc  ^= 0xA001;
      } else {                     // Else LSB is not set
        crc >>= 1;                 // Just shift right
      }
    }
  }
  return crc;
}

uint32_t ModbusRTU_struct::readTypeId() {
  return read_32b_InputRegister(25);
}

uint32_t ModbusRTU_struct::readSensorId() {
  return read_32b_InputRegister(29);
}

uint8_t ModbusRTU_struct::getLastError() const {
  return _last_error;
}

uint32_t ModbusRTU_struct::getFailedReadsSinceLastValid() const {
  return _reads_nodata;
}

void ModbusRTU_struct::startWrite() {
  // transmit to device  -> DE Enable, /RE Disable (for control MAX485)
  if ((_dere_pin == -1) || !isInitialized()) { return; }
  digitalWrite(_dere_pin, HIGH);
  delay(2); // Switching may take some time
}

void ModbusRTU_struct::startRead() {
  if (!isInitialized()) { return; }
  easySerial->flush(); // clear out tx buffer

  // receive from device -> DE Disable, /RE Enable (for control MAX485)
  if (_dere_pin != -1) {
    digitalWrite(_dere_pin, LOW);
  }
}
