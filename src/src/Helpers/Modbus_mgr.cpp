
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include <ESPeasySerial.h>
# include "Modbus_mgr.h"


// ModbusMGR structure representing the singleton Modbus Management entity
// Thw manager has an overview of all Modbus links and the conneted devices.
// The manager allows multiple Modbus devices to connect to a single Modbus link while supporting multiple links.
// The modbus manager is not involved in the actual data transport, this is handled by a direct relation between Modbus device and
// ModbusLINK object.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
    reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::reset()
{
  _modbus_link->reset();
  delete _modbus_link;
  _modbus_link = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::connect(const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               ModbusLINK_struct     **link,
                               uint8_t                *deviceID) {
  return connect(port, serial_rx, serial_tx, baudrate, -1, false, link, deviceID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::connect(const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               int8_t                  dere_pin,
                               bool                    collision_detect,
                               ModbusLINK_struct     **link,
                               uint8_t                *deviceID) {
  if (_modbus_link == nullptr) {
    _modbus_link = new (std::nothrow) ModbusLINK_struct();
    if (_modbus_link == nullptr) {
      return false; // Memory allocation failed
    }
    if (!_modbus_link->init(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
      delete _modbus_link;
      _modbus_link = nullptr;
      return false; // Initialization failed
    }
    else {
      *link = _modbus_link;
      *deviceID = 1; // First device ID
    }
  }
  return true; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::disconnect(uint8_t deviceID) {
  return false; // TODO: implement
}

#endif // FEAURE_MODBUS
