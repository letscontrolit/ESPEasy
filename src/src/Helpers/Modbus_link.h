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

// Modbus request queue element structure
// This structure represents a single Modbus request and its associated response.
struct Modbus_RequestQueueElement {
  Modbus_RequestQueueElement() = default;

  ModbusTransactionType       _messageType = ModbusTransactionType::NONE;              // Type of Modbus message
  void                       *_userData    = nullptr;                                  // Pointer to user (device) data
  void                       *_userState   = nullptr;                                  // Pointer to user (device) defined state
  uint16_t                    _id          = 0;                                        // ID of the request
  struct ModbusDEVICE_struct *_device      = nullptr;                                  // Pointer to the Modbus device requesting the
                                                                                       // action
  uint16_t _sendframe_length = 0;                                                      // Length of the request frame
  uint16_t _rcvframe_length  = 0;                                                      // Expected length of the response frame
                                                                                       // expected
  enum ModbusQueueState _state                         = ModbusQueueState::NOT_QUEUED; // State of the request exchange
  uint16_t              _timeout                       = 0;                            // Specified timeout value for the request
  unsigned long         _startTime                     = 0;                            // Time the request was issued
  uint8_t               _sendframe[MODBUS_XMIT_BUFFER] = { 0 };                        // Reqest frame to send
  uint8_t               _rcvframe[MODBUS_RCV_BUFFER]   = { 0 };                        // Response frame received

};

// Queue of Modbus request elements
typedef std::list<Modbus_RequestQueueElement *> Modbus_RequestQueue;


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
            const int16_t           serial_rx,
            const int16_t           serial_tx,
            int16_t                 baudrate);

  bool init(const ESPEasySerialPort port,
            const int16_t           serial_rx,
            const int16_t           serial_tx,
            int16_t                 baudrate,
            int8_t                  dere_pin,
            bool                    collision_detect = false);

  bool  isInitialized() const;

  bool  reconfigure(const ESPEasySerialPort port,
                   const int16_t           serial_rx,
                   const int16_t           serial_tx,
                   int16_t                 baudrate,
                   int8_t                  dere_pin,
                   bool                    collision_detect = false);

  Modbus_RequestQueueElement* newTransaction(struct ModbusDEVICE_struct *device);
  bool                        freeTransaction(Modbus_RequestQueueElement *transaction);
  void                        freeTransactions(struct ModbusDEVICE_struct *device);
  uint16_t                    queueTransaction(Modbus_RequestQueueElement *transaction);
  void                        processCommand();

  int16_t                     getBaudrate(void) const;

  ESPEasySerialPort           getPort(void) const;

  int16_t                     getSerialRX(void) const;

  int16_t                     getSerialTX(void) const;

  int8_t                      getDerePin(void) const;

  bool                        getCollisionDetect(void) const;

private:

  static void dumpQueueElement(Modbus_RequestQueueElement *el);
  static void dumpState(ModbusQueueState_t state);

  ESPeasySerial      *_easySerial       = nullptr; // Pointer to the serial port object
  Modbus_RequestQueue _requestQueue     = {};      // Queue of Modbus requests to process
  uint16_t            _queueID          = 0;       // ID for the last request queued
  uint16_t            _modbus_timeout   = 180;     // Default Modbus timeout in milliseconds
  uint32_t            _reads_pass       = 0;       // TODO: statistics
  uint32_t            _reads_crc_failed = 0;       // TODO: statistics
  uint32_t            _reads_nodata     = 0;       // TODO: statistics
  uint8_t             _dere_pin         = 0;       // Pin for RS485 direction control
  bool                _collision_detect = false;   // Flag to indicate if collision detection is enabled

  uint8_t _last_error = 0;

};


#endif // FEATURE_MODBUS
#endif // HELPERS_MODBUS_LINK_H
