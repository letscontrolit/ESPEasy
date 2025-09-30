#ifndef HELPERS_MODBUS_DEVICE_H
#define HELPERS_MODBUS_DEVICE_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include <ESPeasySerial.h>
# include "Modbus_link.h"

# define MODBUS_TRANSMIT_BUFFER  12

typedef enum class ModbusQueueState {
  EMPTY     = 0, // The entry was not found in the queue
  QUEUED    = 1, // The entry is in the queue, but not yet processed
  BUSY      = 2, // The entry is being processed
  AVAILABLE = 3, // The entry has been processed and the result is available
  ERROR     = 4  // An error occurred during processing
} ModbusQueueState_t;

// ModbusDEVICE structure representing a MODBUS Device
// This is a single device that may share it's Modbus link with multiple other devices.
// It uses the ModbusLINKManager to find the ModbusLINK object that handles the data transport.
// It is the ModbusDEVICE that builds the Modbus request frames and parses the responses.
struct ModbusDEVICE_struct  {
private:

  enum class ModbusMessageType {
    NONE                   = 0,
    READ_HOLDING_REGISTERS = 1,
    WRITE_SINGLE_REGISTER  = 2
  };

public:

  ModbusDEVICE_struct() = default;

  ~ModbusDEVICE_struct();

  void reset();

  bool init(uint8_t                 slaveAddress,
            const ESPEasySerialPort port,
            const int16_t           serial_rx,
            const int16_t           serial_tx,
            int16_t                 baudrate);

  bool init(uint8_t                 slaveAddress,
            const ESPEasySerialPort port,
            const int16_t           serial_rx,
            const int16_t           serial_tx,
            int16_t                 baudrate,
            int8_t                  dere_pin,
            bool                    collision_detect = false);

  bool     isInitialized() const;


  void     setModbusTimeout(uint16_t timeout);

  uint16_t getModbusTimeout() const;


  void     processCommand();

  void     linkCallback(uint16_t queueID);

  // Start reading a Modubus holding register. The result will be available later.
  // The function returns true if the request was queued.
  // The state variable will signal the processing state of the request.
  bool readHoldingRegister(uint16_t            address,
                           uint16_t           *valueptr,
                           ModbusQueueState_t *stateptr);

  bool writeSingleRegister(uint16_t            address,
                           uint16_t            value,
                           ModbusQueueState_t *stateptr);

private:

  uint8_t             _modbus_address                    = MODBUS_BROADCAST_ADDRESS;
  ModbusLINK_struct  *_modbus_link                       = nullptr; // Pointer to the Modbus link object
  uint8_t             _deviceID                          = 0;       // Identifier used by the Modbus manager to identify this device
  uint16_t            _queueID                           = 0;       // ID from link identifying the last request
  uint8_t             _sendframe[MODBUS_TRANSMIT_BUFFER] = { 0 };   // STorage for the Modbus request frame
  uint8_t             _sendframe_size                    = 0;       // Size of the actual Modbus request frame
  uint8_t             _recv_buf[MODBUS_RECEIVE_BUFFER]   = { 0 };   // Storage for the Modbus response frame
  uint8_t             _recv_buf_used                     = 0;       // Size of the expected Modbus response frame
  ModbusQueueState_t  _state                             = ModbusQueueState_t::EMPTY;
  uint16_t           *_resultPtr                         = nullptr; // Pointer to the variable to store the result in
  ModbusQueueState_t *_statePtr                          = nullptr; // Pointer to the variable to store the state in
  uint16_t            _timeout                           = 200;     // Timeout value in milliseconds for Modbus requests
  ModbusMessageType   _messageType                       = ModbusMessageType::NONE;

  void buildFrame(uint8_t  slaveAddress,                            // Modbus device slave address
                  uint8_t  functionCode,                            // Modbus function code
                  uint16_t startAddress,                            // Starting address in the Modbus device to read from or write to
                  uint8_t  byteCount);                              // Size of the message in the buffer

  uint16_t CalculateCRC(uint8_t *buf,
                        int      len);

  uint16_t queueFrame();
};

#endif // FEAURE_MODBUS
#endif // HELPERS_MODBUS_LINK_H
