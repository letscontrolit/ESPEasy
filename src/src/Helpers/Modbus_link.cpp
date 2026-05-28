/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODBUS link class
// This class implements a Modbus link over a serial connection. It is part of the Modbus facilities supporting multiple Modbus
// devices on multiple serial Modbus links.
// It supports queuing Modbus requests and responses for multiple Modbus devices sharing the same physical link.
// It exepcts a Modbus device instance to construct and interpret the Modbus messages for the specific device.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include "../Helpers/Modbus_device.h"
# include "../Helpers/Modbus_link.h"

////# define MODBUS_DEBUG
# ifdef BUILD_NO_DEBUG
#  undef MODBUS_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus link class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusLINK_struct::~ModbusLINK_struct() {
  reset();

  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset the ModbusLINK class to initial state
// This aborts all pending transactions and frees the associated resources.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::reset() {
  for (auto it   = _requestQueue.begin(); it != _requestQueue.end(); it++) {
    (*it)->_state = ModbusQueueState::ERROR_OCCURRED;

    if ((*it)->_device != nullptr) {
      (*it)->_device->linkCallback(*it); // Notify the device that the request finished with an error
    }
    delete (*it);                        // destroy the queue element
    it = _requestQueue.erase(it);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the link with the given serial port and parameters, including a dere pin for RS485
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::init(const ESPEasySerialPort port,
                             const int8_t            serial_rx,
                             const int8_t            serial_tx,
                             uint16_t                baudrate,
                             int8_t                  dere_pin,
                             bool                    collision_detect) {
  int available = 0;

  // (re)create the serial port object
  // If the serial port object already exists, delete it first.
  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }
  _easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (_easySerial == nullptr) {
    return false;
  }

  // Set RS485 mode if requested using selected pin for RTS
  const bool rs485Mode = _easySerial->setRS485Mode(dere_pin, collision_detect);
  _easySerial->begin(baudrate);
  _easySerial->flush();
  available = _easySerial->available();

  if (available > 0) {
    // Clear any pending input
    for (int i = available; i > 0; --i) {
      _easySerial->read();
    }
  }
  _dere_pin         = dere_pin;
  _collision_detect = collision_detect;

  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log =
      strformat(F(
                  "---> Modbus: Link %s, Init serial, RX pin %d, TX pin %d, RS485 pin %d, baudrate %d, collision detection %s, RS485 mode %s"),
                ESPEasySerialPort_toString(port),
                serial_rx,
                serial_tx,
                dere_pin,
                baudrate,
                collision_detect ? F("enabled") : F("disabled"),
                rs485Mode ? F("enabled") : F("disabled")
                );
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // MODBUS_DEBUG
  _initialized = true;
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Free all queued transactions for the given device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::freeTransactions(ModbusDEVICE_struct *device)
{
  if (!isInitialized()) {
    addLogMove(LOG_LEVEL_ERROR, F("Modbus: Link, Attempt to free transactions on uninitialized link"));
    return;
  }

  for ( auto it = _requestQueue.begin(); it != _requestQueue.end(); ++it ) {
    if ((*it)->_device == device) {
      (*it)->_state = ModbusQueueState::READY_FOR_DESTROY; // Mark to be destroyed
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue a Modbus transaction. The request is appended to the queue and assigned a unique identifier.
// The client can use this identifier to retrieve the response later.
// This function will transfer ownership of the transaction to the Modbus link, which is responsible for freeing
// the associated resources when the transaction is completed.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ModbusLINK_struct::queueTransaction(Modbus_RequestQueueElement *transaction) {
  if (transaction == nullptr) {
    addLogMove(LOG_LEVEL_ERROR, F("Modbus: Link, Attempt to queue null transaction"));
    return 0;
  }

  if (!isInitialized()) {
    addLogMove(LOG_LEVEL_ERROR, F("Modbus: Link, Attempt to queue transaction on uninitialized link"));
    delete transaction; // Free the transaction since it won't be queued
    return 0;
  }

  if (transaction->_rcvframe_length > MODBUS_RCV_BUFFER) {
    addLogMove(LOG_LEVEL_ERROR, F("Modbus: Link, receive buffer too large"));
    delete transaction; // Free the transaction since it won't be queued
    return 0;
  }

  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus: Link, Queueing transaction ID %u, state %u"), transaction->_id, static_cast<uint>(transaction->_state)));
  }
  # endif // MODBUS_DEBUG
  transaction->_id      = ++(_queueID);             // Assign a unique ID to the transaction
  transaction->_timeout = _modbus_timeout;          // Default timeout value
  transaction->_state   = ModbusQueueState::QUEUED; // Transaction is now queued
  _requestQueue.push_back(transaction);             // Append the request to the queue

  if (!_processing) {                               // Prevent reentrancy issues
    processCommand();                               // Trigger processing of the command queue
  }
  return transaction->_id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the next action to take to process the queue
// This function shall be called periodically to keep the Modbus link active
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::processCommand()
{
  _processing = true;                // Set processing flag to prevent reentrancy issues

  if (!isInitialized() || (_requestQueue.empty())) {
    return;                          // Serial port not initialized or queue is empty, nothing to process
  }

  auto it   = _requestQueue.begin(); // Iterator for the request queue
  bool busy = false;                 // Only process one request at a time

  while ((it != _requestQueue.end()) && !busy && ((*it) != nullptr)) {
    # ifdef MODBUS_DEBUG
    dumpQueueElement(*it);
    # endif // MODBUS_DEBUG

    switch  ((*it)->_state)
    {
      case ModbusQueueState::QUEUED:
      {
        // Send the request
        int available = _easySerial->available();

        if (available > 0) {
          // Clear any pending input
          for (int i = available; i > 0; --i) {
            _easySerial->read();
          }
        }
        _easySerial->write((*it)->_sendframe, (*it)->_sendframe_length);
        (*it)->_state     = ModbusQueueState::MESSAGE_SENT; // Mark as sent, waiting for response
        (*it)->_startTime = millis();                       // Record the time the transaction
        busy              = true;                           // Only process one request at a time
        break;
      }

      case ModbusQueueState::MESSAGE_SENT:
      {
        // Waiting for response
        if (_easySerial->available() >= (*it)->_rcvframe_length) {
          _easySerial->readBytes((*it)->_rcvframe, (*it)->_rcvframe_length);
          (*it)->_state = ModbusQueueState::RESPONSE_RECEIVED; // Mark as response received
        }
        else if (timePassedSince((*it)->_startTime) > (*it)->_timeout) {
          // Timeout expired
          (*it)->_state = ModbusQueueState::ERROR_OCCURRED; // Mark as error
        }
        else {
          // Still waiting
          busy = true;                           // Only process one request at a time
        }

        if (!busy) {                             // We received a response or an error occurred, process the result
          if ((*it)->_device != nullptr) {
            (*it)->_device->linkCallback((*it)); // Notify the device that a response was received
          }

          delete (*it);                          // destroy the queue element
          it = _requestQueue.erase(it);          // Remove it from the list
        }
        else {
          it++;                                  // Move to the next transaction in the queue
        }
        break;
      }

      case ModbusQueueState::NOT_QUEUED: // This state should not be on the queue
      {
        it++;                            // Not queued yet, move to the next transaction in the queue
      }
      default:
      {
        delete (*it);                 // destroy the queue element
        it = _requestQueue.erase(it); // Remove it from the list
        break;
      }
    }                                 // switch
  }                                   // next iterarion

  _processing = false;                // Clear processing flag to allow new processing cycles
  return;
}

int16_t ModbusLINK_struct::getBaudrate(void) const
{
  return _easySerial != nullptr ? _easySerial->getBaudRate() : 0;
}

int16_t ModbusLINK_struct::getSerialRX(void) const
{
  return _easySerial != nullptr ? _easySerial->getRxPin() : -1;
}

int16_t ModbusLINK_struct::getSerialTX(void) const
{
  return _easySerial != nullptr ? _easySerial->getTxPin() : -1;
}

int8_t ModbusLINK_struct::getDerePin(void) const
{
  return _dere_pin;
}

bool ModbusLINK_struct::getCollisionDetect(void) const
{
  return _collision_detect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debugging function to dump the queue element contents to the log
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::dumpQueueElement(Modbus_RequestQueueElement *el) {
  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = strformat(F("Modbus: [ID=%u, Device=%p, State="), el->_id, el->_device);
    log += toString(el->_state);
    log += F(", TX=(");

    for (int i = 0; i < el->_sendframe_length; i++) {
      log += String(el->_sendframe[i], HEX);

      if (i < el->_sendframe_length - 1) {
        log += F(",");
      }
    }
    log += F("), RX=(");

    for (int i = 0; i < el->_rcvframe_length; i++) {
      log += String(el->_rcvframe[i], HEX);

      if (i < el->_rcvframe_length - 1) {
        log += F(",");
      }
    }
    log += F(")] ");
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // MODBUS_DEBUG
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debugging function to dump the queue element state to the log
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::dumpState(ModbusQueueState_t state) {
  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("Modbus: Link, State= "), toString(state)));
  }
  # endif // MODBUS_DEBUG
}

const __FlashStringHelper* toString(ModbusQueueState_t state) {
  switch  (state)
  {
    case ModbusQueueState::NOT_QUEUED:
      return F("NOT_QUEUED");
    case ModbusQueueState::QUEUED:
      return F("QUEUED");
    case ModbusQueueState::MESSAGE_SENT:
      return F("MESSAGE_SENT");
    case ModbusQueueState::RESPONSE_RECEIVED:
      return F("RESPONSE_RECEIVED");
    case ModbusQueueState::ERROR_OCCURRED:
      return F("ERROR_OCCURRED");
    case ModbusQueueState::READY_FOR_DESTROY:
      return F("READY_FOR_DESTROY");
  }
  return F("<error>");
}

#endif // if FEATURE_MODBUS
