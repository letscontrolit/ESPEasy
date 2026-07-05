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
  for (auto it   = _transactionQueue.begin(); it != _transactionQueue.end(); it++) {
    (*it)._state = ModbusQueueState::ERROR_OCCURRED;

    if ((*it)._device != nullptr) {
      (*it)._device->linkCallback(&(*it)); // Notify the device that the request finished with an error
    }

    ////delete (*it);                        // destroy the queue element
    it = _transactionQueue.erase(it);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the link with the given serial port and parameters, including a dere pin for RS485
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::init(const ESPEasySerialPort port,
                             const int8_t            serial_rx,
                             const int8_t            serial_tx,
                             uint32_t                baudrate,
                             int8_t                  dere_pin,
                             bool                    collision_detect) {
  int available = 0;

  _initialized = false; // In case of reinitialization the flag might already been set

  // (re)create the serial port object
  // If the serial port object already exists, delete it first.
  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }

  if (port == ESPEasySerialPort::not_set) {
    return true; // No port configured, report this as a successful initialization of a not connected link
    // Note that the link object exists, but is flagged as not initialized and has no serial port object
  }
  else {
    _easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);
  }

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
                  "Modbus Link: Init link=%s, RX pin %d, TX pin %d, RS485 pin %d, baudrate %d, collision detection %s, RS485 mode %s"),
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
// This shall be called when a device is removed to free the pending requests for the device preventing callback issues.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::freeTransactions(const ModbusDEVICE_struct *device)
{
  if (!isInitialized()) {
    addLogMove(LOG_LEVEL_ERROR, F("Modbus Link: Attempt to free transactions on uninitialized link"));
    return;
  }

  for ( auto it = _transactionQueue.begin(); it != _transactionQueue.end(); ++it ) {
    if ((*it)._device == device) {
      (*it)._state = ModbusQueueState::READY_FOR_DESTROY; // Mark to be destroyed
    }
  }
  processQueue();                                         // Trigger processing of the command queue to free the marked transactions
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a new transaction. The transaction is added to the queue
// The transaction is not queued for processing yet.
// After filling the details of the transaction, the client shall call queueTransaction() to queue it for processing.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Modbus_transaction_ptr ModbusLINK_struct::newTransaction(ModbusDEVICE_struct *device)
{
  if (!isInitialized()) {
    addLogMove(LOG_LEVEL_ERROR, strformat(F("Modbus Link: Attempt to create transaction for uninitialized link %d"), this->_linkId));
    return nullptr;
  }
  else {
    auto& tr = _transactionQueue.emplace_back(device); // Put a new transaction at the end of the queue
    tr._timeout = _modbus_timeout;                     // Default timeout value
    # ifdef MODBUS_DEBUG
    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus Link: new transaction link=%d, transaction ID %u, device %p"), this->_linkId, tr._id, device));
    # endif // MODBUS_DEBUG
    return &tr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue a Modbus transaction. The request is appended to the queue and assigned a unique identifier.
// The client can use this identifier to retrieve the response later.
// This function will transfer ownership of the transaction to the Modbus link, which is responsible for freeing
// the associated resources when the transaction is completed.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::queueTransaction(Modbus_Transaction *transaction) {
  bool success = false;

  if (transaction == nullptr) {
    addLogMove(LOG_LEVEL_ERROR, strformat(F("Modbus Link: Attempt to queue null transaction for link %d"), this->_linkId));
  } else if (!isInitialized()) {
    addLogMove(LOG_LEVEL_ERROR, strformat(F("Modbus Link: Attempt to queue transaction on uninitialized link %d"), this->_linkId));
    transaction->_state = ModbusQueueState::ERROR_OCCURRED; // Mark as error
  }
  else if (transaction->_rcvframe_length > MODBUS_RCV_BUFFER) {
    addLogMove(LOG_LEVEL_ERROR, strformat(F("Modbus Link: receive buffer too large for link %d"), this->_linkId));
    transaction->_state = ModbusQueueState::ERROR_OCCURRED; // Mark as error
  }
  else {
    transaction->_state = ModbusQueueState::QUEUED;         // Transaction is now queued
    success             = true;
  }
  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO) && (transaction != nullptr)) {
    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus link: link=%d, Queueing transaction ID %u, state %s"), this->_linkId, transaction->_id,
                         toString(transaction->_state)));
  }
  # endif // MODBUS_DEBUG

  processQueue(); // Trigger processing of the command queue
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the next action to take to process the queue
// This function shall be called periodically to keep the Modbus link active
// The queue is ordered by the order of queuing, and max one transaction is processed at a time.
// _activeTransaction points to the transaction currently being processed, or nullptr if no transaction is active.
// If there is an active transaction it is handled first. 
// If the link is not busy with an active transaction the queue is examined
// Elements are examined in order until either
//  - an element is found that can be sent, which is then sent and set as active
//  - an element is found that is marked for destruction, which is then removed from the queue
//  - the queue is empty and nothing is done
//
// Note that a timeout for a transaction assumes no late response message will be received.
// If a late response is received, it most likely interferes with the next transaction queued. That transaction most likely fails.
// A rough mitigation is to wait for a full timeout if the next transaction also fails
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::processQueue()
{
  if (_processing || !isInitialized() || (_transactionQueue.empty())) {
    return;           // Serial port not initialized or queue is empty, nothing to process
  }
  _processing = true; // Set processing flag to prevent reentrancy issues

  // Check if there is an active transaction and check if it can be completed
  if (_activeTransaction != nullptr) {

    switch (_activeTransaction->_state)
    {
      case ModbusQueueState::MESSAGE_SENT:
      {

        bool responseOK = true;                     // Assume response message is OK unless proven otherwise
        bool linkBusy   = false;                    // Assume we are done unless proven otherwise
        int  available  = _easySerial->available(); // Cache the number of bytes available in the serial buffer

        // Waiting for response
        if (available >= (_activeTransaction->_rcvframe_length)) {
          _easySerial->readBytes(_activeTransaction->_rcvframe, _activeTransaction->_rcvframe_length);
          _activeTransaction->_state = ModbusQueueState::RESPONSE_RECEIVED; // Mark as response received
        }
        else if (timePassedSince(_activeTransaction->_startTime) > _activeTransaction->_timeout) {
          // Timeout expired
          _activeTransaction->_state = ModbusQueueState::ERROR_OCCURRED; // Mark transaction as error
          _prev_transact_failed      = true;                             // Mark link that the previous transaction failed
          # ifdef MODBUS_DEBUG
          addLogMove(LOG_LEVEL_INFO,
                     strformat(F("Modbus link ERROR: link=%d, transaction ID= %u, available= %d, expected= %d"), this->_linkId,
                               _activeTransaction->_id, available, _activeTransaction->_rcvframe_length));
          # endif // MODBUS_DEBUG
        }
        else {
          // Still waiting
          linkBusy = true; // Only process one request at a time
        }

        if (!linkBusy) {   // We received a response or an error occurred,

          // process the result
          if (_activeTransaction->_device != nullptr) {
            responseOK = _activeTransaction->_device->linkCallback(_activeTransaction); // Notify the device that a response was
                                                                                        // received
          }
          else {
            addLogMove(LOG_LEVEL_ERROR, F("Modbus link: ERROR, Null pointer for device in active transaction"));
          }

          if (_prev_transact_failed && !responseOK) {
            _activeTransaction->_state = ModbusQueueState::RECOVERING; // Wait till timeout to recover from the error
            # ifdef MODBUS_DEBUG
            addLogMove(LOG_LEVEL_INFO,
                       strformat(F("Modbus link RECOVERING: link=%d, transaction ID= %u"), this->_linkId, _activeTransaction->_id));
            # endif // MODBUS_DEBUG
          }
          else {
            _activeTransaction->_state = ModbusQueueState::READY_FOR_DESTROY;
            # ifdef MODBUS_DEBUG
            addLogMove(LOG_LEVEL_INFO,
                       strformat(F("Modbus link FINISHED: link=%d, transaction ID= %u"), this->_linkId,
                                 _activeTransaction->_id));
            # endif // MODBUS_DEBUG
          }
        }
      }

      // Waiting for the timeout to recover from potential link errors.
      case ModbusQueueState::RECOVERING:
      {
        if (timePassedSince(_activeTransaction->_startTime) > _activeTransaction->_timeout) {
          // Timeout expired
          _prev_transact_failed      = false; // Clear the previous transaction failed flag
          _activeTransaction->_state = ModbusQueueState::READY_FOR_DESTROY;
          # ifdef MODBUS_DEBUG
          addLogMove(LOG_LEVEL_INFO,
                     strformat(F("Modbus link RECOVERY FINISHED: link=%d, transaction ID= %u"), this->_linkId,
                               _activeTransaction->_id));
          # endif // MODBUS_DEBUG
        }
        break;
      }

      // Transaction should already be marked for destruction and activeTransaction should have been cleared
      // The activeTransaction can be set to READY_FOR_DESTROY if the device discarded it
      case ModbusQueueState::READY_FOR_DESTROY:
      case ModbusQueueState::ERROR_OCCURRED:
      case ModbusQueueState::RESPONSE_RECEIVED:
      {
        _activeTransaction->_state = ModbusQueueState::READY_FOR_DESTROY;
        break;
      }

      // This is an error condition. activeTransaction should not point to a transaction in this state.
      // Set the activeTransaction to nullptr to allow processing of the next transaction in the queue.
      case ModbusQueueState::NOT_QUEUED:
      case ModbusQueueState::QUEUED:
      {
        _activeTransaction->_state = ModbusQueueState::READY_FOR_DESTROY;
      }
    }
  }

  // If there is no active transaction, check the queue for the next transaction to process
  if ((_activeTransaction == nullptr) || (_activeTransaction->_state == ModbusQueueState::READY_FOR_DESTROY)) {
    auto it = _transactionQueue.begin(); // Iterator for the request queue

    // Main loop to process the queue.
    // Note that the iterator is explicitly incremented in the loop, or implicitly when a transaction is removed from the queue.
    while (it != _transactionQueue.end())  {

      switch  (it->_state)
      {
        // Transaction is created but not yet queued. Skip it untill it is queued or discarded.
        case ModbusQueueState::NOT_QUEUED:
        {
          it++; // Not queued yet, move to the next transaction in the queue
          break;
        }

        // Transaction in queue ready to be transmitted.
        // If there is no active transaction yet, send the request and mark it as active.
        case ModbusQueueState::QUEUED:
        {
          if (_activeTransaction == nullptr) { // Pick only the first rtansaction from the queue to process

            // Clear the serial input buffer before sending the request
            int available = _easySerial->available();

            if (available > 0) {
              // Clear any pending input
              for (int i = available; i > 0; --i) {
                _easySerial->read();
              }
            }
            _easySerial->write(it->_sendframe, it->_sendframe_length);
            it->_state         = ModbusQueueState::MESSAGE_SENT; // Mark as sent, waiting for response
            it->_startTime     = millis();                       // Record the time the transaction
            _activeTransaction = &(*it);                         // Mark this transaction as active
             # ifdef MODBUS_DEBUG
            addLogMove(LOG_LEVEL_INFO,
                       strformat(F("Modbus link: PICKING link=%d, transaction ID=%u, state=%s"), this->_linkId, it->_id,
                                 toString(it->_state)));
            # endif // MODBUS_DEBUG
          }
          it++; // Move to the next transaction in the queue
          break;
        }

        // Transaction is sent, waiting for response. Handled above in the active Transaction processing section.
        case ModbusQueueState::MESSAGE_SENT:
        // Transaction failed in suspicious conditions, Handled above in the active Transaction processing section.
        case ModbusQueueState::RECOVERING:
        {
          it++; // Move to the next transaction in the queue
          break;
        }

        // All remaining states indicate that the transaction can be removed from the queue and destroyed.
        case ModbusQueueState::READY_FOR_DESTROY:
        case ModbusQueueState::ERROR_OCCURRED:              // Not exepected
        case ModbusQueueState::RESPONSE_RECEIVED:           // Not expected
        {
          if (_activeTransaction == &(*it)) {
            _activeTransaction = nullptr;                   // Clear the active transaction if it is being removed from the queue
          }
          it->_state = ModbusQueueState::READY_FOR_DESTROY; // For safety
          it         = _transactionQueue.erase(it);         // Remove it from the list
          break;
        }
      }                                                     // switch
    }                                                       // next iterarion
  }                                                         // check for inactive link conditions
  _processing = false;                                      // Clear processing flag to allow new processing cycles
  return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int16_t ModbusLINK_struct::getBaudrate() const
{
  return _easySerial != nullptr ? _easySerial->getBaudRate() : 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int16_t ModbusLINK_struct::getSerialRX() const
{
  return _easySerial != nullptr ? _easySerial->getRxPin() : -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int16_t ModbusLINK_struct::getSerialTX() const
{
  return _easySerial != nullptr ? _easySerial->getTxPin() : -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int8_t ModbusLINK_struct::getDerePin() const
{
  return _dere_pin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusLINK_struct::getCollisionDetect() const
{
  return _collision_detect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debugging function to dump the queue element state to the log
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusLINK_struct::dumpState(ModbusQueueState_t state) {
  # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("Modbus Link: State= "), toString(state)));
  }
  # endif // MODBUS_DEBUG
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const __FlashStringHelper* toString(ModbusQueueState_t state) {
  # ifdef MODBUS_DEBUG

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
    case ModbusQueueState::RECOVERING:
      return F("RECOVERING");
  }
  # endif // MODBUS_DEBUG
  return F("<error>");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debugging function to dump the queue element contents to the log
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Modbus_Transaction::print()
{
    # ifdef MODBUS_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = strformat(F("Modbus Transaction %p: [ID=%u, Device=%p, State="), this, _id, _device);
    log += toString(_state);
    log += F(", TX=(");

    for (int i = 0; i < _sendframe_length; i++) {
      log += String(_sendframe[i], HEX);

      if (i < _sendframe_length - 1) {
        log += F(",");
      }
    }
    log += F("), RX=(");

    for (int i = 0; i < _rcvframe_length; i++) {
      log += String(_rcvframe[i], HEX);

      if (i < _rcvframe_length - 1) {
        log += F(",");
      }
    }
    log += F(")] ");
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // MODBUS_DEBUG
}

#endif // if FEATURE_MODBUS
