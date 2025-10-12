
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include "Modbus_device.h"
# include "modbus_link.h"
# include "modbus_mgr.h"

const uint8_t MODBUS_READ_HOLDING_REGISTERS = 0x03;
const uint8_t MODBUS_READ_INPUT_REGISTERS = 0x04;
const uint8_t MODBUS_WRITE_SINGLE_REGISTER = 0x06;
const uint8_t MODBUS_WRITE_MULTIPLE_REGISTERS = 0x10;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusDEVICE_struct::~ModbusDEVICE_struct() {
  reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::reset() {
  if (_modbus_link != nullptr) {
    _modbus_link->freeTransactions(this);
    ModbusMGR_singleton.disconnect(_deviceID);
    _modbus_link = nullptr;
  }
  _deviceID = 0;

  _modbus_address = MODBUS_BROADCAST_ADDRESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::init(uint8_t                 slaveAddress,
                               const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate) {
  return init(slaveAddress, port, serial_rx, serial_tx, baudrate, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::init(uint8_t                 slaveAddress,
                               const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               int8_t                  dere_pin,
                               bool                    collision_detect) {
  // Request the Modbus manager to connect this device to a Modbus link with the given parameters.
  bool success = ModbusMGR_singleton.connect(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect, &_modbus_link, &_deviceID);

  _modbus_address = slaveAddress;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("---> ModbusDevice Init: Slave address = ");
    log += slaveAddress;
    log += F(", This = ");
    log += (size_t)this;
    log += F(", deviceID  = ");
    log += _deviceID;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  // TODO: further implementation needed
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::isInitialized() const {
  return (_modbus_link != nullptr) && (_modbus_link->isInitialized());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::setModbusTimeout(uint16_t timeout) {
  _timeout = timeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusDEVICE_struct::getModbusTimeout() const
{
  return _timeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modubus holding register. The result will be available later.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readHoldingRegister(uint16_t            address,
                                              uint16_t           *valuePtr,
                                              ModbusResultState_t *statePtr)
{
  if (_modbus_link == nullptr) {
    return false;
  }
  Modbus_RequestQueueElement *request = _modbus_link->newTransaction(this);

  request->_messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  request->_userData     = valuePtr;
  request->_sendframe[0] = _modbus_address;
  request->_sendframe[1] = MODBUS_READ_HOLDING_REGISTERS;
  request->_sendframe[2] = highByte(address);
  request->_sendframe[3] = lowByte(address);
  request->_sendframe[4] = 0;
  request->_sendframe[5] = 1;                 // Read 1 register
  uint16_t crc = CalculateCRC(request->_sendframe, 6);
  request->_sendframe[6]     = lowByte(crc);  // CRC low byte
  request->_sendframe[7]     = highByte(crc); // CRC high byte
  request->_sendframe_length = 8;             // Size with CRC
  request->_rcvframe_length  = 7;             // Expect 8 bytes in response
  dump_buffer(request->_sendframe, request->_sendframe_length);
  uint16_t queueID = _modbus_link->queueRequest(request);
  *statePtr = ModbusResultState::BUSY;

  // Don't touch *valueptr here, it might contain a previous valid result.
  return false; // TODO: implement
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::writeSingleRegister(uint16_t            address,
                                              uint16_t            value,
                                              ModbusResultState_t *statePtr)
{
  Modbus_RequestQueueElement *request =    _modbus_link->newTransaction(this);

  request->_sendframe[0] = _modbus_address;
  request->_sendframe[1] = MODBUS_WRITE_SINGLE_REGISTER;
  request->_sendframe[2] = highByte(address);
  request->_sendframe[3] = lowByte(address);
  request->_sendframe[4] = highByte(value);
  request->_sendframe[5] = lowByte(value);
  uint16_t crc = CalculateCRC(request->_sendframe, 6);
  request->_sendframe[6]     = lowByte(crc);  // CRC low byte
  request->_sendframe[7]     = highByte(crc); // CRC high byte
  request->_sendframe_length = 8;             // Size with CRC
  request->_rcvframe_length  = 8;             // Expect 8 bytes in response
  uint16_t queueID = _modbus_link->queueRequest(request);
  *statePtr             = ModbusResultState::BUSY;
  request->_messageType = ModbusTransactionType::WRITE_SINGLE_REGISTER;
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::processCommand() {
  if (_modbus_link != nullptr) {
    _modbus_link->processCommand(); // Trigger processing of the command queue on the link
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback function called by the Modbus link when a response is received for a queued request.
// Note that the response might be an invalid response or a timeout
// The queueID identifies the request.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::linkCallback(Modbus_RequestQueueElement *req)
{
  String log = F("---> Device callback: ");

  if (req == nullptr) {
    log += F("ERROR: Null pointer passed");
    return;
  }
  log += req->_id;
  log += F(", Message = ");
  log += static_cast<uint8_t>(req->_messageType);


  switch (req->_messageType) {
    case ModbusTransactionType::READ_HOLDING_REGISTERS:
    {
      if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_READ_HOLDING_REGISTERS) && (req->_rcvframe[2] == 2)) {
        uint16_t crc = CalculateCRC(req->_rcvframe, 5);

        if ((req->_rcvframe[5] == lowByte(crc)) && (req->_rcvframe[6] == highByte(crc))) {
          // Valid response
          if (req->_userData != nullptr) {
            *((uint16_t *)req->_userData) = (req->_rcvframe[3] << 8) | req->_rcvframe[4]; // Combine high and low byte
          }
        } else {
          // Invalid CRC
        }
      } else {
        // Invalid response
      }
      break;
    }

    case ModbusTransactionType::WRITE_SINGLE_REGISTER:
    {
      if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_READ_HOLDING_REGISTERS) && (req->_rcvframe[2] == 2)) {
        uint16_t crc = CalculateCRC(req->_rcvframe, 5);

        if ((req->_rcvframe[5] == lowByte(crc)) && (req->_rcvframe[6] == highByte(crc))) {
          // Valid response
        }
        else {
          // Invalid CRC
        }
      } else {
        // Invalid response
      }
      break;
    }

    case ModbusTransactionType::NONE:
    {
      // Should not happen
      break;
    }

    default:
    {
      // Unknown message type
      break;
    }
  }

  _modbus_link->freeTransaction(req);
  addLogMove(LOG_LEVEL_INFO, log);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute the Modbus RTU CRC
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusDEVICE_struct::CalculateCRC(uint8_t *buf, int len) {
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];     // XOR uint8_t into least sig. uint8_t of crc

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::dump_buffer(const uint8_t *buffer, size_t length) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("---> Modbus: Dumping buffer: ");

    for (size_t i = 0; i < length; ++i) {
      log += String(buffer[i], HEX);

      if (i < length - 1) {
        log += F(", ");
      }
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

#endif // if FEATURE_MODBUS
