#ifndef HELPERS_MODBUS_DEVICE_H
#define HELPERS_MODBUS_DEVICE_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "Modbus_link.h"

# ifndef MODBUS_BROADCAST_ADDRESS
#  define MODBUS_BROADCAST_ADDRESS 0xFE // Address used for boardcast messages
# endif // ifndef MODBUS_BROADCAST_ADDRESS

// States for the Modbus queue elements
enum class ModbusResultState {
  Busy    = 0, // Transaction is not completed
  Success = 1, // Transaction successfully completed
  Error   = 2, // Transaction completed with an error
};

// ModbusDEVICE structure representing a MODBUS Device
// This is a single device that may share it's Modbus link with multiple other devices.
// It uses the ModbusLINKManager to find the ModbusLINK object that handles the data transport.
// It is the ModbusDEVICE that builds the Modbus request frames and parses the responses.
struct ModbusDEVICE_struct  {
private:

public:

  ModbusDEVICE_struct() = default;

  ~ModbusDEVICE_struct();

  void reset();

bool init( uint8_t                 slaveAddress,
           int                     linkId);

  bool     isInitialized() const;

  void     setModbusTimeout(uint16_t timeout);

  uint16_t getModbusTimeout() const;

  void     processCommand();

  void     linkCallback(Modbus_RequestQueueElement *transaction);

  // Start reading a Modubus holding register. The result will be available later.
  // The function returns true if the request was queued.
  // The state variable will signal the processing state of the request.
  bool readHoldingRegister(uint16_t             address,
                           uint16_t            *valueptr,
                           ModbusResultState *stateptr);

  bool writeSingleRegister(uint16_t             address,
                           uint16_t             value,
                           ModbusResultState *stateptr);
                           
  bool readModuleHoldingRegister(uint8_t              busAddress,
                                 uint16_t             registerAddress,
                                 uint16_t            *valuePtr,
                                 ModbusResultState *statePtr);

private:

  uint8_t            _modbus_address = MODBUS_BROADCAST_ADDRESS;
  ModbusLINK_struct *_modbus_link    = nullptr; // Pointer to the Modbus link object
  uint8_t            _deviceID       = 0;       // Identifier used by the Modbus manager to identify this device
  uint16_t           _timeout        = 200;     // Timeout value in milliseconds for Modbus requests

  static uint16_t CalculateCRC(uint8_t *buf,
                               int      len);
  static void     dump_buffer(const uint8_t *buffer,
                              size_t         length);
};

#endif // FEAURE_MODBUS_FAC
#endif // HELPERS_MODBUS_LINK_H
