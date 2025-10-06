#ifndef HELPERS_MODBUS_LINK_H
#define HELPERS_MODBUS_LINK_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>
# include "MODBUS_RTU.h"

# define MODBUS_XMIT_BUFFER  12
# define MODBUS_RCV_BUFFER   256

// Forward declaration of ModbusDEVICE_struct to avoid circular dependency issues
struct ModbusDEVICE_struct;

typedef enum class ModbusQueueState {
  NOT_QUEUED        = 0,
  QUEUED            = 1,
  MESSAGE_SENT      = 2,
  RESPONSE_RECEIVED = 3,
  ERROR_OCCURRED    = 4,
  READY_FOR_DESTROY = 5
} ModbusQueueState_t;

enum class ModbusMessageType {
  NONE                   = 0,
  READ_HOLDING_REGISTERS = 1,
  WRITE_SINGLE_REGISTER  = 2
};

// Modbus request queue element structure
// This structure represents a single Modbus request and its associated response.
struct Modbus_RequestQueueElement {
  Modbus_RequestQueueElement(uint16_t id, ModbusQueueState state)
    : _id(id),
    _state(state)
  {}

  ModbusMessageType           _messageType = ModbusMessageType::NONE;                  // Type of Modbus message
  void                       *_userData    = nullptr;                                  // Pointer to user data
  uint16_t                    _id          = 0;                                        // ID of the request
  struct ModbusDEVICE_struct *_device      = nullptr;                                  // Pointer to the Modbus device requesting the
                                                                                       // action
  uint16_t _sendframe_length = 0;                                                      // Length of the request frame
  uint16_t _rcvframe_length  = 0;                                                      // Expected length of the response frame
                                                                                       // expected
  enum ModbusQueueState _state                         = ModbusQueueState::NOT_QUEUED; // State of the request exchange
  uint16_t              _timeout                       = 0;                            // Specified timeout value for the request
  unsigned long         _deadline                      = 0;                            // Timeout deadline for the request
  uint8_t               _sendframe[MODBUS_XMIT_BUFFER] = { 0 };                        // Reqest frame to send
  uint8_t               _rcvframe[MODBUS_RCV_BUFFER]   = { 0 };                        // Response frame received
};

// Queue of Modbus request elements
typedef std::list<Modbus_RequestQueueElement> Modbus_RequestQueue;


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

  bool                        isInitialized() const;

  Modbus_RequestQueueElement* newTransaction(struct ModbusDEVICE_struct *device);
  bool                        freeTransaction(Modbus_RequestQueueElement *transaction);
  void                        freeTransactions(struct ModbusDEVICE_struct *device);
  uint16_t                    queueRequest(Modbus_RequestQueueElement *transaction);
  bool                        getResponse(uint16_t                     id,
                                          Modbus_RequestQueueElement **transaction);

  void                        processCommand();

private:

  static void dumpQueueElement(Modbus_RequestQueueElement *el);

  ESPeasySerial      *_easySerial       = nullptr; // Pointer to the serial port object
  int8_t              _dere_pin         = -1;      // Pin to control DE/RE of RS485 transceiver
  Modbus_RequestQueue _requestQueue     = {};      // Queue of Modbus requests to process
  uint16_t            _queueID          = 0;       // ID for the last request queued
  uint32_t            _reads_pass       = 0;
  uint32_t            _reads_crc_failed = 0;
  uint32_t            _reads_nodata     = 0;       // This will be reset as soon as a valid packet has been received.
  uint16_t            _modbus_timeout   = 180;
  uint8_t             _last_error       = 0;
};


#endif // FEATURE_MODBUS
#endif // HELPERS_MODBUS_LINK_H
