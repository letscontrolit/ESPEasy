#ifndef HELPERS_MODBUS_MGR_H
#define HELPERS_MODBUS_MGR_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

# include <ESPeasySerial.h>
# include "Modbus_link.h"


// ModbusMGR structure representing the singleton Modbus Management entity
// Thw manager has an overview of all Modbus links and the conneted devices.
// The manager allows multiple Modbus devices to connect to a single Modbus link while supporting multiple links.
// The modbus manager is not involved in the actual data transport, this is handled by a direct relation between Modbus device and
// ModbusLINK object.
struct ModbusMGR_struct  {
  ModbusMGR_struct() = default;

  ~ModbusMGR_struct();

  void reset();

  bool connect(const ESPEasySerialPort port,
               const int16_t           serial_rx,
               const int16_t           serial_tx,
               int16_t                 baudrate,
               ModbusLINK_struct      **link,
               uint8_t                *deviceID);

  bool connect(const ESPEasySerialPort port,
               const int16_t           serial_rx,
               const int16_t           serial_tx,
               int16_t                 baudrate,
               int8_t                  dere_pin,
               bool                    collision_detect,
               ModbusLINK_struct      **link,
               uint8_t                *deviceID);

  bool disconnect(uint8_t deviceID);

private:

  ModbusLINK_struct *_modbus_link = nullptr; // Pointer to the Modbus link object
};

static struct ModbusMGR_struct ModbusMGR_singleton = {}; // Singleton instance of the Modbus Manager

#endif // FEAURE_MODBUS
#endif // HELPERS_MODBUS_MGR_H
