/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODBUS device class
// This class implements a single Modbus device connected over a serial link. 
// It is part of the Modbus facilities supporting multiple Modbus devices on multiple serial Modbus links.
// It supports queuing Modbus requests and responses for multiple Modbus devices sharing the same physical link.
// The Modbus device class will interpret the Modbus messages for the connected hardware and queue it at the link class.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include "../Helpers/Modbus_device.h"
# include "../Helpers/Modbus_mgr.h"
#include "Modbus_device.h"

# define MODBUS_DEBUG
# ifdef BUILD_NO_DEBUG
#  undef MODBUS_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

const uint8_t MODBUS_READ_HOLDING_REGISTERS   = 0x03;
const uint8_t MODBUS_READ_INPUT_REGISTERS     = 0x04;
const uint8_t MODBUS_WRITE_SINGLE_REGISTER    = 0x06;
const uint8_t MODBUS_WRITE_MULTIPLE_REGISTERS = 0x10;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus device class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusDEVICE_struct::~ModbusDEVICE_struct() {
  if (_modbus_link != nullptr) {
    _modbus_link->freeTransactions(this); // Make sure all queued transactions for this device are freed to prevent callbacks to a
                                          // destructed object
  }
  ModbusMGR_singleton.disconnect(_deviceID);
  _modbus_link    = nullptr;
  _deviceID       = 0;
  _modbus_address = MODBUS_BROADCAST_ADDRESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset the Modbus device class to initial state
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::reset() {
  if (_modbus_link != nullptr) {
    _modbus_link->freeTransactions(this); // Make sure all queued transactions for this device are freed to prevent callbacks to a
                                          // destructed object
  }
  ModbusMGR_singleton.disconnect(_deviceID);
  _modbus_link    = nullptr;
  _deviceID       = 0;
  _modbus_address = MODBUS_BROADCAST_ADDRESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization connected to an existing link. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::init(uint8_t slaveAddress, int linkId)
{
   bool success = ModbusMGR_singleton.connect(linkId, &_modbus_link, &_deviceID);
  _modbus_address = slaveAddress;
  # ifdef MODBUS_DEBUG 
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus: Device Init, Slave address = %u, This = %p, deviceID = %u, linkId=%d"), slaveAddress, this, _deviceID, linkId));
  }
  # endif // MODBUS_DEBUG
    return success;
}
// Checker for device class initialization status
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::isInitialized() const {
  return (_modbus_link != nullptr) && (_modbus_link->isInitialized());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set the Modbus timeout value for this device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::setModbusTimeout(uint16_t timeout) {
  _timeout = timeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve the Modbus timeout value for this device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusDEVICE_struct::getModbusTimeout() const
{
  return _timeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modubus holding register. The result will be available later.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readHoldingRegister(uint16_t             address,
                                              uint16_t            *valuePtr,
                                              ModbusResultState *statePtr) {
  return readModuleHoldingRegister(_modbus_address, address, valuePtr, statePtr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modbus holding register from another module on the bus. The result will be available later.
// The function returns true if the request was queued.
// Note: This function accesses registers from other devices on the same Modbus bus. 
//       This should be used with care to prevent conflicts. This is beyond the intended scope of the Modbus device class.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readModuleHoldingRegister(uint8_t              busAddress,
                                                    uint16_t             registerAddress,
                                                    uint16_t            *valuePtr,
                                                    ModbusResultState *statePtr)
{
  if (_modbus_link == nullptr) {
    return false;
  }
  Modbus_RequestQueueElement *request = _modbus_link->newTransaction(this);

  request->_messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  request->_userData     = valuePtr;
  request->_userState    = statePtr;
  request->_sendframe[0] = busAddress;
  request->_sendframe[1] = MODBUS_READ_HOLDING_REGISTERS;
  request->_sendframe[2] = highByte(registerAddress);
  request->_sendframe[3] = lowByte(registerAddress);
  request->_sendframe[4] = 0;
  request->_sendframe[5] = 1;                 // Read 1 register
  uint16_t crc = CalculateCRC(request->_sendframe, 6);
  request->_sendframe[6]     = lowByte(crc);  // CRC low byte
  request->_sendframe[7]     = highByte(crc); // CRC high byte
  request->_sendframe_length = 8;             // Size with CRC
  request->_rcvframe_length  = 7;             // Expect 8 bytes in response
  ////dump_buffer(request->_sendframe, request->_sendframe_length);
  uint16_t queueID = _modbus_link->queueTransaction(request);
  *statePtr = ModbusResultState::Busy;

  // Don't touch *valueptr here, it might contain a previous valid result still to be handled.
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start writing a Modbus single register.
// The function returns true if the request was queued. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::writeSingleRegister(uint16_t             address,
                                              uint16_t             value,
                                              ModbusResultState *statePtr)
{
  if (_modbus_link == nullptr) {
    return false;
  }
  Modbus_RequestQueueElement *request =    _modbus_link->newTransaction(this);

  request->_messageType = ModbusTransactionType::WRITE_SINGLE_REGISTER;
  request->_userState   = statePtr;

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
  ////dump_buffer(request->_sendframe, request->_sendframe_length);
  uint16_t queueID = _modbus_link->queueTransaction(request);
  *statePtr = ModbusResultState::Busy;

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Periodic processing function to allow the Modbus device to process its queued requests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::processCommand() {
  // TODO: This function is currently not used since the Modbus link processing is triggered directly from the scheduler.
  if (_modbus_link != nullptr) {
    ////_modbus_link->processCommand(); // Trigger processing of the command queue on the link
    ////ModbusMGR_singleton.processLinks();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback function called by the Modbus link when a response is received for a queued request.
// Note that the response might be an invalid response or a timeout
// The queueID identifies the request.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::linkCallback(Modbus_RequestQueueElement *req)
{
  ModbusResultState resultState = ModbusResultState::Error; // Default to error unless proven otherwise

  if (req == nullptr) {
    addLogMove(LOG_LEVEL_INFO, F("Modbus: ERROR, Null pointer passed in callback"));
    return;
  }

  # ifdef MODBUS_DEBUG
  String log = strformat(F("Modbus: Device callback, device= %d, Request= %d, Message= %d"),
                         _deviceID,
                         req->_id,
                         static_cast<uint8_t>(req->_messageType)
                         );
  # endif // MODBUS_DEBUG

  if (req->_state == ModbusQueueState::ERROR_OCCURRED) {
    # ifdef MODBUS_DEBUG
    log += F(" Link error occurred");
    # endif // MODBUS_DEBUG
  }
  else {
    switch (req->_messageType) {
      case ModbusTransactionType::READ_HOLDING_REGISTERS:
      {
        if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_READ_HOLDING_REGISTERS) && (req->_rcvframe[2] == 2)) {
          uint16_t crc = CalculateCRC(req->_rcvframe, 5);

          if ((req->_rcvframe[5] == lowByte(crc)) && (req->_rcvframe[6] == highByte(crc))) {
            // Valid response
            if (req->_userData != nullptr) {
              *(static_cast<uint16_t *>(req->_userData)) = (req->_rcvframe[3] << 8) | req->_rcvframe[4]; // Combine high and low byte
              resultState                                = ModbusResultState::Success;
            }
          }
        }
        break;
      }

      case ModbusTransactionType::WRITE_SINGLE_REGISTER:
      {
        if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_READ_HOLDING_REGISTERS) && (req->_rcvframe[2] == 2)) {
          uint16_t crc = CalculateCRC(req->_rcvframe, 5);

          if ((req->_rcvframe[5] == lowByte(crc)) && (req->_rcvframe[6] == highByte(crc))) {
            resultState = ModbusResultState::Success;
          }
        }
        break;
      }

      case ModbusTransactionType::NONE:
      {
        // Error condition, this transaction type should not be queued
      # ifdef MODBUS_DEBUG
        log += F(" Invalid transaction type");
      # endif // MODBUS_DEBUG
        break;
      }

      default:
      {
        // Error condition, missed a transaction type
      # ifdef MODBUS_DEBUG
        log += F(" Unknown transaction type");
      # endif // MODBUS_DEBUG
        break;
      }
    }
  }

  *(static_cast<ModbusResultState *>(req->_userState)) = resultState;
  _modbus_link->freeTransaction(req);
  # ifdef MODBUS_DEBUG
  log += F(", Result = ");
  log += (resultState == ModbusResultState::Success) ? F("SUCCESS") : F("ERROR");
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // MODBUS_DEBUG
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
// Debugging function to dump the buffer contents to the log
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::dump_buffer(const uint8_t *buffer, size_t length) {
  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Modbus: Device, Dumping buffer: ");

    for (size_t i = 0; i < length; ++i) {
      log += String(buffer[i], HEX);

      if (i < length - 1) {
        log += F(", ");
      }
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // MODBUS_DEBUG
}

#endif // if FEATURE_MODBUS
