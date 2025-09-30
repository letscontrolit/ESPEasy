/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODBUS link class
// This class implements a Modbus link over a serial connection.
// It supports queuing Modbus requests and responses for multiple Modbus devices sharing the same physical link.
// It exepcts a Modbus device instance to construct and interpret the Modbus messages for the specific device.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include "Modbus_device.h"
# include "Modbus_link.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusLINK_struct::~ModbusLINK_struct() {
  reset();

  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset the ModbusLINK structure
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::reset() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the link with the given serial port and parameters
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::init(const ESPEasySerialPort port,
                             const int16_t           serial_rx,
                             const int16_t           serial_tx,
                             int16_t                 baudrate) {
  return init(port, serial_rx, serial_tx, baudrate, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the link with the given serial port and parameters, including a dere pin for RS485
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::init(const ESPEasySerialPort port,
                             const int16_t           serial_rx,
                             const int16_t           serial_tx,
                             int16_t                 baudrate,
                             int8_t                  dere_pin,
                             bool                    collision_detect) {
  // (re)create the serial port object
  // If the serial port object already exists, delete it first.
  if (ModbusLINK_struct::_easySerial != nullptr) {
    delete ModbusLINK_struct::_easySerial;
    ModbusLINK_struct::_easySerial = nullptr;
  }
  ModbusLINK_struct::_easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (ModbusLINK_struct::_easySerial == nullptr) {
    return false;
  }

  // Set RS485 mode if requested using selected pin for RTS
  bool rs485Mode = ModbusLINK_struct::_easySerial->setRS485Mode(dere_pin, collision_detect);

  ModbusLINK_struct::_easySerial->begin(baudrate);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("P183: Init serial: RX pin ");
    log += serial_rx;
    log += F(", TX pin ");
    log += serial_tx;
    log += F(", RS485 mode selected on pin ");
    log += dere_pin;
    log += F(", baudrate ");
    log += baudrate;
    log += F(", collision detection ");
    log += collision_detect ? F("enabled") : F("disabled");
    log += F(", RS485mode enabled: ");
    log += rs485Mode ? F("yes") : F("no");
    addLogMove(LOG_LEVEL_DEBUG, log);
  }

  return true;
}

// Return the initialization status of the link
bool ModbusLINK_struct::isInitialized() const {
  return _easySerial != nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue a Modbus request. The request is appended to the request and assigned a unique identifier.
// The client can use this identifier to retrieve the response later.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusLINK_struct::queueRequest(struct ModbusDEVICE_struct *device,
                                         uint8_t                    *sendframe,
                                         uint16_t                    sendframe_length,
                                         uint16_t                    rcvframe_length,
                                         uint16_t                    timeout) {
  struct Modbus_RequestQueueElement *req = new Modbus_RequestQueueElement(_queueID, 0);

  req->_id     = ++(_queueID);                 // Assign a unique ID to the request
  req->_device = device;                       // The Modbus device making the request

  for (int i = 0; i < sendframe_length; ++i) { // Make a copy of the sendframe to persist it in teh queue element
    req->_sendframe[i] = sendframe[i];
  }
  req->_sendframe_length = sendframe_length;
  req->_rcvframe_length  = rcvframe_length;
  req->_timeout          = timeout;
  req->_state            = 0;    // Initial state

  _requestQueue.push_back(*req); // Append the request to the queue
  return req->_id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if there is a response available for the given request ID and retrieve it if available
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::getResponse(uint16_t id, uint8_t *rcvframe, uint16_t rcvframe_length) {
  for (auto it = _requestQueue.begin(); it != _requestQueue.end();) {
    if (it->_id == id) {                                   // Found the request with the matching ID
      if (it->_state == 1) {                               // Response is ready
        if (rcvframe_length >= it->_rcvframe_length) {
          for (int i = 0; i < it->_rcvframe_length; ++i) { // Copy the response to the provided buffer
            rcvframe[i] = it->_rcvframe[i];
          }
          it = _requestQueue.erase(it);                    // Remove the request from the queue after retrieving the response
          return true;                                     // Provided buffer is too small
        } else {
          return false;
        }
      } else { // Response not ready yet
        return false;
      }
    } else {
      ++it;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove a request from the queue based on its ID
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::removeRequest(uint16_t id) {
  for (auto it = _requestQueue.begin(); it != _requestQueue.end();) {
    if (it->_id == id) {
      it = _requestQueue.erase(it);
    } else {
      ++it;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the next action to take to process the queue
// This function shall be called periodically to keep the Modbus link active
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t ModbusLINK_struct::processCommand()
{
  if (_easySerial == nullptr) {
    return 0; // Serial port not initialized
  }

  for (auto it = _requestQueue.begin(); it != _requestQueue.end(); it++) {
    if (it->_state == 0) {
      // Send the request
      if (_easySerial->available() > 0) {
        // Clear any pending input
        for (int i = _easySerial->available(); i > 0; --i) {
          _easySerial->read();
        }
      }

      _easySerial->write(it->_sendframe, it->_sendframe_length);
      it->_state    = 1;                       // Mark as sent, waiting for response
      it->_deadline = millis() + it->_timeout; // Record the dealine value for the response
      return 1;                                // Indicate that a request was sent
    }

    if (it->_state == 1) {
      // Waiting for response
      if (_easySerial->available() >= it->_rcvframe_length) {
        _easySerial->readBytes(it->_rcvframe, it->_rcvframe_length);
        it->_state = 2;                       // Mark as response received

        if (it->_device != nullptr) {
          it->_device->linkCallback(it->_id); // Notify the device that a response was received
        }
      }
      else if (millis() > it->_deadline) {
        // Timeout expired
        it->_state = 3;                       // Mark as error

        if (it->_device != nullptr) {
          it->_device->linkCallback(it->_id); // Notify the device that a response was received
          return 2;
        }
        else {
          return 0; // Still waiting
        }
      }
    }
  }
  return 0;
}

#endif // if FEATURE_MODBUS
