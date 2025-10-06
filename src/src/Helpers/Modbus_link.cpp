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
// Provide a new transaction structure that can be used to build a Modbus request and queue it at this link
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Modbus_RequestQueueElement * ModbusLINK_struct::newTransaction(struct ModbusDEVICE_struct *device)
{
  Modbus_RequestQueueElement *req = new Modbus_RequestQueueElement(0, ModbusQueueState::NOT_QUEUED);

  if (req != nullptr) {
    req->_id      = ++(_queueID);                 // Assign a unique ID to the transaction
    req->_device  = device;                       // The Modbus device making the request
    req->_timeout = _modbus_timeout;              // Default timeout value
    req->_state   = ModbusQueueState::NOT_QUEUED; // Initial state
    return req;
  }
  else {
    return nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Free a previously allocated transaction structure
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::freeTransaction(Modbus_RequestQueueElement *transaction) {
  String log = F("---> Free: ");

  if (transaction != nullptr) {
    transaction->_state = ModbusQueueState::READY_FOR_DESTROY; // Mark as freed
    log                += transaction->_id;
    addLogMove(LOG_LEVEL_INFO, log);
    return true;
  }
  else {
    log += F("Attempt to free null transaction");
    addLogMove(LOG_LEVEL_INFO, log);
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Free all queued transactions for the given device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::freeTransactions(ModbusDEVICE_struct * device)
{
  for ( auto it   = _requestQueue.begin(); it != _requestQueue.end(); ++it ) {
    if (it->_device == device) {
      it->_state = ModbusQueueState::READY_FOR_DESTROY; // Mark to be destroyed
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue a Modbus request. The request is appended to the request and assigned a unique identifier.
// The client can use this identifier to retrieve the response later.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusLINK_struct::queueRequest(Modbus_RequestQueueElement *transaction) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("---> Modbus queue request: request ID = ");
    log += transaction->_id;
    log += F(", State = ");
    log += uint(transaction->_state);
    addLogMove(LOG_LEVEL_INFO, log);
  }
  transaction->_state = ModbusQueueState::QUEUED; // Initial state
  _requestQueue.push_back(*transaction);          // Append the request to the queue
  processCommand();                               // Trigger processing of the command queue
  return transaction->_id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if there is a response available for the given request ID and retrieve it if available
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::getResponse(uint16_t id, Modbus_RequestQueueElement **transaction) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the next action to take to process the queue
// This function shall be called periodically to keep the Modbus link active
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::processCommand()
{
  String log = F("---> Process queue: ");

  if (_easySerial == nullptr) {
    log += F("Serial port not initialized");
    addLogMove(LOG_LEVEL_INFO, log);
    return;                          // Serial port not initialized
  }

  auto it   = _requestQueue.begin(); // Iterator for the request queue
  bool busy = false;                 // Only process one request at a time

  while ((it != _requestQueue.end()) && !busy) {
    dumpQueueElement(&(*it));

    switch  (it->_state) {
      case ModbusQueueState::QUEUED:
      {
        log += F(" state QUEUED, ID = ");
        log += it->_id;

        // Send the request
        if (_easySerial->available() > 0) {
          // Clear any pending input
          for (int i = _easySerial->available(); i > 0; --i) {
            _easySerial->read();
          }
        }

        _easySerial->write(it->_sendframe, it->_sendframe_length);
        it->_state    = ModbusQueueState::MESSAGE_SENT; // Mark as sent, waiting for response
        it->_deadline = millis() + it->_timeout;        // Record the dealine value for the response
        log          += F(" state QUEUED, ID = ");
        log          += it->_id;
        busy          = true;                           // Only process one request at a time
        break;
      }

      case ModbusQueueState::MESSAGE_SENT:
      {
        log += F(" state MESSAGE_SENT, ID = ");
        log += it->_id;

        // Waiting for response
        if (_easySerial->available() >= it->_rcvframe_length) {
          _easySerial->readBytes(it->_rcvframe, it->_rcvframe_length);
          it->_state = ModbusQueueState::RESPONSE_RECEIVED; // Mark as response received

          if (it->_device != nullptr) {
            it->_device->linkCallback(&(*it));              // Notify the device that a response was received
          }
        }
        else if (millis() > it->_deadline) {
          // Timeout expired
          it->_state = ModbusQueueState::ERROR_OCCURRED; // Mark as error
          log       += F(" Timeout ");

          if (it->_device != nullptr) {
            it->_device->linkCallback(&(*it)); // Notify the device that a response was received
          }
          else {
            log += F(" Available=");
            log += _easySerial->available();
          }
          it++;
        }
        else {
          // Still waiting
          busy = true; // Only process one request at a time
        }
        break;
      }

      case ModbusQueueState::ERROR_OCCURRED:
      {
        log += F(" state ERROR_OCCURRED, ID = ");
        log += it->_id;

        it++;
        break;
      }

      case ModbusQueueState::READY_FOR_DESTROY:
      {
        log += F(" state READY_FOR_DESTROY, ID = ");
        log += it->_id;

        it = _requestQueue.erase(it);
        break;
      }

      default:
        it++;
        break;
    } // switch
    addLogMove(LOG_LEVEL_INFO, log);
  }   // next iterarion

  return;
}

void ModbusLINK_struct::dumpQueueElement(Modbus_RequestQueueElement *el) {
  String log = F("[ ID=");

  log += el->_id;
  log += F(", Device=");
  log += String((uint32_t)(el->_device), HEX);
  log += F(", State=");
  log += (uint)el->_state;
  log += F(", TX=");

  for (int i = 0; i < el->_sendframe_length; i++) {
    log += String(el->_sendframe[i], HEX);
    log += F(",");
  }
  log += F(", RX=");

  for (int i = 0; i < el->_rcvframe_length; i++) {
    log += String(el->_rcvframe[i], HEX);
    log += F(",");
  }
  log += F("] ");
  addLogMove(LOG_LEVEL_INFO, log);
}

#endif // if FEATURE_MODBUS
