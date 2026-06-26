#ifndef HELPERS_MODBUS_LINK_H
#define HELPERS_MODBUS_LINK_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>

# define MODBUS_XMIT_BUFFER  12
# define MODBUS_RCV_BUFFER   256

// Forward declaration of ModbusDEVICE_struct to avoid circular dependency issues
struct ModbusDEVICE_struct;

// States for the Modbus queue elements
typedef enum class ModbusQueueState {
  NOT_QUEUED        = 0, // Initial state, element is created but not yet queued
  QUEUED            = 1, // Element is queued and waiting to be processed
  MESSAGE_SENT      = 2, // Request message has been sent, waiting for response
  RESPONSE_RECEIVED = 3, // Response has been received and is being processed
  ERROR_OCCURRED    = 4, // An error occurred during processing (e.g., timeout, invalid response)
  READY_FOR_DESTROY = 5  // Element is marked for deletion and can be freed

} ModbusQueueState_t;

const __FlashStringHelper* toString(ModbusQueueState_t state);

// Types of Modbus transactions supported by the Modbuss_device
// This enumeration is used by the Modbus device to indicate which transaction is associated with the queue element.
// See Modbus specification for details on function codes.
typedef enum class ModbusTransactionType {
  NONE                   = 0, // Undefined/unknown transaction type
  READ_HOLDING_REGISTERS = 1, // Read holding registers (function code 0x03)
  WRITE_SINGLE_REGISTER  = 2  // Write single register (function code 0x06)

} ModbusTransactionType_t;

// Modbus transaction structure
// This structure represents a single Modbus request and its associated response.
struct Modbus_Transaction {
  Modbus_Transaction() = default;
  Modbus_Transaction(ModbusDEVICE_struct *device)
  {
    _device = device;
  }

  void print();                                                                        // Print the transaction details for debugging

  ModbusTransactionType _messageType = ModbusTransactionType::NONE;                    // Type of Modbus message
  void                 *_userData    = nullptr;                                        // Pointer to user (device) data
  void                 *_userState   = nullptr;                                        // Pointer to user (device) defined state
  uint16_t              _userId      = 0;                                              // Client defined identifier for this transaction,
                                                                                       // can be used to match responses to requests
  uint16_t                    _id     = 0;                                             // Unique ID of the request (for debugging)
  struct ModbusDEVICE_struct *_device = nullptr;                                       // Pointer to the Modbus device requesting the
                                                                                       // action
  uint16_t              _sendframe_length              = 0;                            // Length of the request frame
  uint16_t              _rcvframe_length               = 0;                            // Expected length of the response frame
  enum ModbusQueueState _state                         = ModbusQueueState::NOT_QUEUED; // State of the request exchange
  uint16_t              _timeout                       = 0;                            // Specified timeout value for the request
  unsigned long         _startTime                     = 0;                            // Time the request was issued
  uint8_t               _sendframe[MODBUS_XMIT_BUFFER] = { 0 };                        // Reqest frame to send
  uint8_t               _rcvframe[MODBUS_RCV_BUFFER]   = { 0 };                        // Response frame received

};

typedef Modbus_Transaction *Modbus_transaction_ptr;

// Queue of Modbus request elements
typedef std::list<Modbus_Transaction> Modbus_TransactionQueue;


// ModbusLINK structure representing a MODBUS LINK
// This is a single serial link that can have multiple Modbus devices conected to it.
// It is used by the ModbusLINKManager to manage multiple links.
// Each ModbusLINK can have multiple ModbusDEVICE_struct instances representing the devices on the link.
// The ModbusLINK structure maintains a queue of Modbus requests and associated responses.
struct ModbusLINK_struct  {
  ModbusLINK_struct() = default;

  ~ModbusLINK_struct();

  void reset();

  bool init(const ESPEasySerialPort port,
            const int8_t            serial_rx,
            const int8_t            serial_tx,
            uint32_t                baudrate,
            int8_t                  dere_pin,
            bool                    collision_detect = false);

  bool                   isInitialized() const { return (_easySerial != nullptr) && _initialized; }

  void                   freeTransactions(const ModbusDEVICE_struct *device);
  Modbus_transaction_ptr newTransaction(ModbusDEVICE_struct *device);
  bool                   queueTransaction(Modbus_Transaction *transaction);
  void                   processQueue();

  int16_t                getBaudrate() const;
  int16_t                getSerialRX() const;
  int16_t                getSerialTX() const;
  int8_t                 getDerePin() const;
  bool                   getCollisionDetect() const;

private:

  static void dumpState(ModbusQueueState_t state);

  ESPeasySerial          *_easySerial       = nullptr; // Pointer to the serial port object
  Modbus_TransactionQueue _transactionQueue = {};      // Queue of Modbus transactions to process
  uint16_t                _queueID          = 0;       // ID for the last request queued
  uint16_t                _modbus_timeout   = 180;     // Default Modbus timeout in milliseconds

  uint8_t _dere_pin         = 0;                       // Pin for RS485 direction control
  bool    _collision_detect = false;                   // Flag to indicate if collision detection is enabled
  bool    _initialized      = false;
  bool    _processing       = false;                   // Flag to indicate if the command queue is currently being processed, used to
                                                       // prevent
                                                       // reentrancy issues

};


#endif // FEATURE_MODBUS
#endif // HELPERS_MODBUS_LINK_H
