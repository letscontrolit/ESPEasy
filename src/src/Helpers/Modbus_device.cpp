
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include "Modbus_device.h"
# include "modbus_link.h"
# include "modbus_mgr.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusDEVICE_struct::~ModbusDEVICE_struct() {
  reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::reset() {
  if (_modbus_link != nullptr) {
    ModbusMGR_singleton.disconnect(_deviceID);
    _modbus_link = nullptr;
  }
  _deviceID       = 0;
  _queueID        = 0;
  _sendframe_size = 0;
  _recv_buf_used  = 0;
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
                                              ModbusQueueState_t *statePtr) {
  buildFrame(_modbus_address, MODBUS_READ_HOLDING_REGISTERS, address, 1);
  uint16_t crc = CalculateCRC(_sendframe, _sendframe_size);
  _sendframe[_sendframe_size++] = lowByte(crc);  // CRC low byte
  _sendframe[_sendframe_size++] = highByte(crc); // CRC high byte
  _queueID                      = queueFrame();
  *statePtr                     = ModbusQueueState_t::QUEUED;
  _statePtr                     = statePtr;
  _resultPtr                    = valuePtr;
  _state                        = ModbusQueueState_t::QUEUED;
  _messageType                  = ModbusMessageType::READ_HOLDING_REGISTERS;

  // Don't touch *valueptr here, it might contain a previous valid result.
  return false; // TODO: implement
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusDEVICE_struct::writeSingleRegister(uint16_t            address,
                                              uint16_t            value,
                                              ModbusQueueState_t *statePtr)
{
  buildFrame(_modbus_address, MODBUS_WRITE_SINGLE_REGISTER, address, 1);
  _sendframe[4] = highByte(value);
  _sendframe[5] = lowByte(value);
  uint16_t crc = CalculateCRC(_sendframe, _sendframe_size);
  _sendframe[_sendframe_size++] = lowByte(crc);  // CRC low byte
  _sendframe[_sendframe_size++] = highByte(crc); // CRC high byte
  _queueID                      = queueFrame();
  *statePtr                     = ModbusQueueState_t::QUEUED;
  _statePtr                     = statePtr;
  _state                        = ModbusQueueState_t::QUEUED;
  _messageType                  = ModbusMessageType::WRITE_SINGLE_REGISTER;
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::processCommand() {
  if (_modbus_link != nullptr) {
    _modbus_link->processCommand(); // Trigger processing of the command queue on the link
  }
  else {
    _state = ModbusQueueState_t::ERROR;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback function called by the Modbus link when a response is received for a queued request.
// Note that the response might be an invalid response or a timeout
// The queueID identifies the request.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::linkCallback(uint16_t queueID)
{
  if (queueID != _queueID) {
    return; // Not for us
  }
  bool response = _modbus_link->getResponse(_queueID, _recv_buf, _recv_buf_used);

  if (response) {
    switch (_messageType) {
      case ModbusMessageType::READ_HOLDING_REGISTERS:
      {
        if ((_recv_buf[0] == _modbus_address) && (_recv_buf[1] == MODBUS_READ_HOLDING_REGISTERS) && (_recv_buf[2] == 2)) {
          uint16_t crc = CalculateCRC(_recv_buf, 5);

          if ((_recv_buf[5] == lowByte(crc)) && (_recv_buf[6] == highByte(crc))) {
            // Valid response
            if (_resultPtr != nullptr) {
              *_resultPtr = (_recv_buf[3] << 8) | _recv_buf[4]; // Combine high and low byte
            }
            _state = ModbusQueueState_t::AVAILABLE;
          } else {
            // Invalid CRC
            _state = ModbusQueueState_t::ERROR;
          }
        } else {
          // Invalid response
          _state = ModbusQueueState_t::ERROR;
        }
        return;
        break;
      }
      case ModbusMessageType::WRITE_SINGLE_REGISTER:
      {
        if ((_recv_buf[0] == _modbus_address) && (_recv_buf[1] == MODBUS_READ_HOLDING_REGISTERS) && (_recv_buf[2] == 2)) {
          uint16_t crc = CalculateCRC(_recv_buf, 5);

          if ((_recv_buf[5] == lowByte(crc)) && (_recv_buf[6] == highByte(crc))) {
            // Valid response
            _state = ModbusQueueState_t::AVAILABLE;
          }
          else {
            // Invalid CRC
            _state = ModbusQueueState_t::ERROR;
          }
        } else {
          // Invalid response
          _state = ModbusQueueState_t::ERROR;
        }
        break;
      }
      case ModbusMessageType::NONE:
      {
        // Should not happen
        _state = ModbusQueueState_t::ERROR;
        break;
      }

      default:
      {
        // Unknown message type
        _state = ModbusQueueState_t::ERROR;
        break;
      }
    }
  }

  // Update the state as seen by the client
  if (_statePtr != nullptr) {
    *_statePtr = _state;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Build a Modbus RTU frame filling in the standard fields.
// Note that thsi does not include the CRC.
// The frame is stored in the _sendframe buffer and the size is stored in _sendframe_size.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusDEVICE_struct::buildFrame(uint8_t  slaveAddress,
                                     uint8_t  functionCode,
                                     uint16_t startAddress,
                                     uint8_t  byteCount) {
  _sendframe[0]   = slaveAddress;
  _sendframe[1]   = functionCode;
  _sendframe[2]   = highByte(startAddress);
  _sendframe[3]   = lowByte(startAddress);
  _sendframe[4]   = highByte(byteCount);
  _sendframe[5]   = lowByte(byteCount);
  _sendframe_size = 6; // Size without the CRC
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
// Queue the assembled Modbus frame for transmission over the link using the ModbusLink object.
// The function returns the queue ID assigned to the request, or 0 if queuing failed
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusDEVICE_struct::queueFrame() {
  if (_modbus_link == nullptr) {
    return false;
  }
  _queueID = _modbus_link->queueRequest(this, _sendframe, _sendframe_size, MODBUS_RECEIVE_BUFFER, _timeout);
  return _queueID;
}

#endif // if FEATURE_MODBUS
