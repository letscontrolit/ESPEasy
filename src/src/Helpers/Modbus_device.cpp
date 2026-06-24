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

////# define MODBUS_DEBUG
# ifdef BUILD_NO_DEBUG
#  undef MODBUS_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

// Modbus function codes, see Modbus specification
const uint8_t MODBUS_READ_HOLDING_REGISTERS   = 0x03;
const uint8_t MODBUS_READ_INPUT_REGISTERS     = 0x04;
const uint8_t MODBUS_WRITE_SINGLE_REGISTER    = 0x06;
const uint8_t MODBUS_WRITE_MULTIPLE_REGISTERS = 0x10;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus device class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusDEVICE_struct::~ModbusDEVICE_struct() {

  ModbusMGR_singleton.disconnect(_linkId, this);
  _deviceID       = 0;
  _modbus_address = MODBUS_BROADCAST_ADDRESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset the Modbus device class to initial state
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::reset() {

  ModbusMGR_singleton.disconnect(_linkId, this);
  _deviceID       = 0;
  _modbus_address = MODBUS_BROADCAST_ADDRESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initializae the Modbus device and connect it to the given Modbus link
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::init(uint8_t slaveAddress, int linkId, taskIndex_t taskIndex)
{
  bool success = ModbusMGR_singleton.connect(linkId, this);

  _modbus_address = slaveAddress;
  _taskIndex      = taskIndex;
  _linkId         = linkId;

  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus Device: Init, Slave address = %u, This = %p, deviceID = %u, linkId=%d, taskIndex=%d"),
                         slaveAddress, this,  _deviceID, linkId, taskIndex));
  }
  # endif // MODBUS_DEBUG
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checker for device class initialization status
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::isInitialized() const {
  return (_linkId >= 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set the Modbus timeout value for this device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::setModbusTimeout(uint16_t timeout) { _timeout = timeout; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve the Modbus timeout value for this device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusDEVICE_struct::getModbusTimeout() const
{
  return _timeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modubus holding register. The result will be available later through the provided pointers.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readHoldingRegister(uint16_t          address,
                                              uint16_t         *valuePtr,
                                              ModbusResultState*statePtr)
{
  if (!isInitialized()) {
    return false;
  }
  Modbus_Transaction *transaction = nullptr; // Declare the request pointer
  ModbusMGR_singleton.newTransaction(_linkId, this, transaction);

  if (transaction == nullptr) {
    return false;                                                // Failed to allocate a request structure
  }
  transaction->_messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  transaction->_userId      = 0; // Not used for this type of request
  transaction->_userData    = valuePtr; 
  transaction->_userState   = statePtr;
  createReadFrame(*transaction, _modbus_address, address);
  (void)ModbusMGR_singleton.queueTransaction(_linkId, transaction ); // Transaction is now owned by the Modbus link
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modubus holding register. The result will be available later through a task event.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readHoldingRegister(uint16_t address, uint16_t uid)
{
  return readModuleHoldingRegister(_modbus_address, address, uid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading a Modbus holding register from another module on the bus. The result will be available later.
// The function returns true if the request was queued.
// Note: This function accesses registers from other devices on the same Modbus bus.
//       This should be used with care to prevent conflicts. This is beyond the intended scope of the Modbus device class.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readModuleHoldingRegister(uint8_t  busAddress,
                                                    uint16_t registerAddress,
                                                    uint16_t uid)
{
  if (!isInitialized()) {
    return false;
  }

  Modbus_Transaction *transaction = nullptr; // Declare the request pointer
  ModbusMGR_singleton.newTransaction(_linkId, this, transaction);

  if (transaction == nullptr) {
    return false;                                                // Failed to allocate a request structure
  }
  transaction->_messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  transaction->_userId      = uid; 
  transaction->_userData    = nullptr; 
  transaction->_userState   = nullptr;
  createReadFrame(*transaction, busAddress, registerAddress);
  (void)ModbusMGR_singleton.queueTransaction(_linkId, transaction ); // Transaction is now owned by the Modbus link
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start reading multiple Modubus holding registers. The result will be available later through a task event.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::readHoldingRegisters(uint16_t address, uint16_t size, uint16_t uid)
{
  if (!isInitialized()) {
    return false;
  }

  Modbus_Transaction *transaction = nullptr; // Declare the request pointer
  ModbusMGR_singleton.newTransaction(_linkId, this, transaction);

  if (transaction == nullptr) {
    return false;                                                // Failed to allocate a request structure
  }
  transaction->_messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  transaction->_userId      = uid; 
  transaction->_userData    = nullptr; 
  transaction->_userState   = nullptr;
  createReadFrame(*transaction, _modbus_address, address, size);
  (void)ModbusMGR_singleton.queueTransaction(_linkId, transaction ); // Transaction is now owned by the Modbus link
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construct a Modbus read holding registers message
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::createReadFrame(Modbus_Transaction& request,
                                          uint8_t             busAddress,
                                          uint16_t            registerAddress,
                                          uint16_t            registerCount)
{
  request._messageType  = ModbusTransactionType::READ_HOLDING_REGISTERS;
  request._sendframe[0] = busAddress;
  request._sendframe[1] = MODBUS_READ_HOLDING_REGISTERS;
  request._sendframe[2] = highByte(registerAddress);
  request._sendframe[3] = lowByte(registerAddress);
  request._sendframe[4] = highByte(registerCount);     // Number of registers to read
  request._sendframe[5] = lowByte(registerCount);
  uint16_t crc = CalculateCRC(request._sendframe, 6);
  request._sendframe[6]     = lowByte(crc);            // CRC low byte
  request._sendframe[7]     = highByte(crc);           // CRC high byte
  request._sendframe_length = 8;                       // Size with CRC
  request._rcvframe_length  = 5 + (2 * registerCount); // Expected # bytes in response
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start writing a Modbus single register.
// The function returns true if the request was queued.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::writeSingleRegister(uint16_t           address,
                                              uint16_t           value,
                                              ModbusResultState *statePtr)
{
  if (!isInitialized()) {
    return false;
  }
  
  Modbus_Transaction *transaction = nullptr; // Declare the request pointer
  ModbusMGR_singleton.newTransaction(_linkId, this, transaction);

  if (transaction == nullptr) {
    return false;                                                // Failed to allocate a request structure
  }
  transaction->_messageType  = ModbusTransactionType::WRITE_SINGLE_REGISTER;
  transaction->_userId      = 0; // Not used for this type of request
  transaction->_userData    = nullptr; 
  transaction->_userState   = statePtr;


  transaction->_sendframe[0] = _modbus_address;
  transaction->_sendframe[1] = MODBUS_WRITE_SINGLE_REGISTER;
  transaction->_sendframe[2] = highByte(address);
  transaction->_sendframe[3] = lowByte(address);
  transaction->_sendframe[4] = highByte(value);
  transaction->_sendframe[5] = lowByte(value);
  uint16_t crc = CalculateCRC(transaction->_sendframe, 6);
  transaction->_sendframe[6]     = lowByte(crc);  // CRC low byte
  transaction->_sendframe[7]     = highByte(crc); // CRC high byte
  transaction->_sendframe_length = 8;             // Size with CRC
  transaction->_rcvframe_length  = 8;             // Expect 8 bytes in response
  (void)ModbusMGR_singleton.queueTransaction(_linkId, transaction); // Transaction is now owned by the Modbus link
  *statePtr = ModbusResultState::Busy;

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process any pending commands for this device.
// Should only be used when polling for an immediate response, not for regular asynchronous requests.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::processCommand(void)
{
  if (isInitialized()) {
    ModbusMGR_singleton.processLinks();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback function called by the Modbus link when a response is received for a queued request.
// Note that the response might be an invalid response or a timeout
// The queueID identifies the request.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::linkCallback(Modbus_Transaction *req)
{
  ModbusResultState resultState = ModbusResultState::Error; // Default to error unless proven otherwise

  if (req == nullptr) {
    addLogMove(LOG_LEVEL_INFO, F("Modbus Device: ERROR, Null pointer passed in callback"));
    return;
  }

  # ifdef MODBUS_DEBUG
  String log = strformat(F("Modbus Device: Device callback, device= %d, link= %d, Request= %d, Message= %d"),
                         _deviceID,
                         _linkId,
                         req->_id,
                         static_cast<uint8_t>(req->_messageType)
                         );
  # endif // MODBUS_DEBUG

  if (req->_state == ModbusQueueState::ERROR_OCCURRED) {
    sendEvent(*req, ModbusResultMessageType::Error, 0, 0, 0);
    # ifdef MODBUS_DEBUG
    log += F(" Link error occurred");
    # endif // MODBUS_DEBUG
  }
  else {
    switch (req->_messageType)
    {
      case ModbusTransactionType::READ_HOLDING_REGISTERS:
      {
        int registerCount = req->_sendframe[4] << 8 | req->_sendframe[5]; // Number of registers requested in the original request

        // Validate the response: Check slave address, function code and byte count
        if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_READ_HOLDING_REGISTERS) &&
            (registerCount == req->_rcvframe[2] >> 1))
        {
          uint16_t crc = CalculateCRC(req->_rcvframe, req->_rcvframe_length - 2);

          if ((req->_rcvframe[req->_rcvframe_length - 2] == lowByte(crc)) && (req->_rcvframe[req->_rcvframe_length - 1] == highByte(crc))) {

            // Check which return mode is used by the client and return the value accordingly
            if (req->_userData != nullptr) {
              // Return value through user data pointer specified by the client when queuing the request
              // Note: this is only the first register value if multiple registers were read. Returning multiple register values through
              // user data pointer is not supported.
              *(static_cast<uint16_t *>(req->_userData)) = (req->_rcvframe[3] << 8) | req->_rcvframe[4];
            }
            else if (registerCount == 1)
            {
              // Return value through an event with one parameter.
              sendEvent(*req, ModbusResultMessageType::SingleValue, (req->_rcvframe[3] << 8) | req->_rcvframe[4]);
            }
            else
            {
              // Return value through an event with a ModbusRegisterSet_struct.

              for (int i = 0; i < registerCount && i < 8; i++) {
                _registerSet.data[i] = (req->_rcvframe[3 + (i * 2)] << 8) | req->_rcvframe[4 + (i * 2)];
              }
              _registerSet.size = registerCount;
              sendEvent(*req, ModbusResultMessageType::MultiValue, &_registerSet);
            }
            resultState = ModbusResultState::Success;
          }
          else {
            # ifdef MODBUS_DEBUG
            log += F(" CRC check failed");
            # endif // ifdef MODBUS_DEBUG
          }

        }
        else {
          # ifdef MODBUS_DEBUG
          log += F(" Invalid response format");
          # endif // ifdef MODBUS_DEBUG
        }
        break;
      }

      case ModbusTransactionType::WRITE_SINGLE_REGISTER:
      {
        if ((req->_rcvframe[0] == _modbus_address) && (req->_rcvframe[1] == MODBUS_WRITE_SINGLE_REGISTER) && (req->_rcvframe[2] == 2)) {
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

  if (req->_userState != nullptr) {
    *(static_cast<ModbusResultState *>(req->_userState)) = resultState;
  }
  # ifdef MODBUS_DEBUG
  log += F(", Result = ");
  log += (resultState == ModbusResultState::Success) ? F("SUCCESS") : F("ERROR");
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // MODBUS_DEBUG
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send a PLUGIN_TASKTIMER_IN event to the task associated with this device.
// This is used by the Modbus link to notify the device of responses received for queued requests.
// This version of the function is used to pass data through parameters in the event.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::sendEvent(Modbus_Transaction    & req,
                                    ModbusResultMessageType messageType,
                                    int                     par2,
                                    int                     par3,
                                    int                     par4,
                                    int                     par5,
                                    int                     par6,
                                    int                     par7,
                                    int                     par8)
{
  struct EventStruct TempEvent;

  TempEvent.Par1      = static_cast<int>(messageType);
  TempEvent.Par2      = par2;
  TempEvent.Par3      = par3;
  TempEvent.Par4      = par4;
  TempEvent.Par5      = par5;
  TempEvent.Par6      = par6;
  TempEvent.Par7      = par7;
  TempEvent.Par8      = par8;
  TempEvent.TaskIndex = _taskIndex;  // Send to the task associated with this device
  TempEvent.idx       = req._userId; // Identifier as specified by the client in the request
  TempEvent.Source    = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;
  String dummy;

  PluginCall(PLUGIN_TASKTIMER_IN, &TempEvent, dummy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send a PLUGIN_TASKTIMER_IN event to the task associated with this device.
// This is used by the Modbus link to notify the device of responses received for queued requests.
// This version of the function is used to pass multiple register values in a ModbusRegisterSet_struct.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::sendEvent(Modbus_Transaction      & req,
                                    ModbusResultMessageType   messageType,
                                    ModbusRegisterSet_struct *registerSet)
{
  struct EventStruct TempEvent;

  TempEvent.Par1      = static_cast<int>(messageType);
  TempEvent.Data      = reinterpret_cast<uint8_t *>(registerSet);
  TempEvent.TaskIndex = _taskIndex;  // Send to the task associated with this device
  TempEvent.idx       = req._userId; // Identifier as specified by the client in the request
  TempEvent.Source    = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;
  String dummy;

  // Note: The ModbusRegisterSet_struct is passed as a pointer in the Data field of the event to avoid a deep copy.
  // This assumes PluginCall will secure that the pointer will remain valid when the event is processed.
  // And the data is consumed when the function returns (No multi-threading)
  PluginCall(PLUGIN_TASKTIMER_IN, &TempEvent, dummy);
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
    String log = F("Modbus Device Dumping buffer: ");

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

#endif // if FEATURE_MODBUS_FAC
