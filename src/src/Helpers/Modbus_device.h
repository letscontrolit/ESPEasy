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

  bool init(uint8_t     slaveAddress,
            int         linkId,
            taskIndex_t taskIndex);

  bool     isInitialized() const;

  void     setModbusTimeout(uint16_t timeout);

  uint16_t getModbusTimeout() const;

  void     linkCallback(Modbus_RequestQueueElement *transaction);

  // Start reading a Modubus holding register. The result will be available later.
  // The function returns true if the request was queued.
  // The state variable will signal the processing state of the request.
  bool readHoldingRegister(uint16_t           address,
                           uint16_t          *valueptr,
                           ModbusResultState *stateptr);

  // Start reading a Modbus holding register with reslt returned through event PLUGIN_TASKTIMER_IN
  // The function returns true if the request was queued.
  // Use uid to identify the request. This value will be passed back in the PLUGIN_TASKTIMER_IN event
  bool readHoldingRegister(uint16_t address,
                           uint16_t uid);

  // Start writing a single Modbus register.
  // The function returns true if the request was queued.
  bool writeSingleRegister(uint16_t           address,
                           uint16_t           value,
                           ModbusResultState *stateptr);

  // Start reading a Modbus holding register from another module. The result will be available later.
  // The function returns true if the request was queued.
  // Note: This function accesses registers from other devices on the same Modbus bus. This is beyond the intended scope of the Modbus
  // device class.
  bool readModuleHoldingRegister(uint8_t  busAddress,
                                 uint16_t registerAddress,
                                 uint16_t uid);

  void processCommand(void);

private:

  uint8_t            _modbus_address = MODBUS_BROADCAST_ADDRESS;
  ModbusLINK_struct *_modbus_link    = nullptr; // Pointer to the Modbus link object
  uint8_t            _deviceID       = 0;       // Identifier used by the Modbus manager to identify this device
  uint16_t           _timeout        = 200;     // Timeout value in milliseconds for Modbus requests
  taskIndex_t        _taskIndex      = 0;       // Task index for sending events to the task associated with this device

  void sendEvent(Modbus_RequestQueueElement *req,
                 int                         par1,
                 int                         par2,
                 int                         par3,
                 int                         par4);

  void createReadFrame(Modbus_RequestQueueElement& request,
                       uint8_t                     busAddress,
                       uint16_t                    registerAddress);

  static uint16_t CalculateCRC(uint8_t *buf,
                               int      len);

  static void     dump_buffer(const uint8_t *buffer,
                              size_t         length);

};

#endif // FEAURE_MODBUS_FAC
#endif // HELPERS_MODBUS_LINK_H
