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
               ModbusLINK_struct     **link,
               uint8_t                *deviceID);

  bool connect(const ESPEasySerialPort port,
               const int16_t           serial_rx,
               const int16_t           serial_tx,
               int16_t                 baudrate,
               int8_t                  dere_pin,
               bool                    collision_detect,
               ModbusLINK_struct     **link,
               uint8_t                *deviceID);

  bool disconnect(uint8_t deviceID);

private:

  struct ModbusLinkInfo_struct {
    ESPEasySerialPort         port             = ESPEasySerialPort::not_set;
    int16_t                   serial_rx        = -1;
    int16_t                   serial_tx        = -1;
    int16_t                   baudrate         = 9600;
    int8_t                    dere_pin         = -1;      // Pin used for RS485 DE/RE control, -1 if not used
    bool                      rs485_mode       = false;   // True if RS485 mode is enabled
    bool                      collision_detect = false;   // True if collision detection is enabled
    struct ModbusLINK_struct *link             = nullptr; // Pointer to the Modbus link object
  };

  struct ModbusDeviceInfo_struct {
    uint8_t                       deviceID = 0;          // Unique ID assigned by the Modbus manager
    struct ModbusDEVICE_struct   *device   = nullptr;    // Pointer to the Modbus device object
    struct ModbusLinkInfo_struct *link     = nullptr;    // Pointer to the Modbus link info
  };

   ModbusLinkInfo_struct   *_modbus_links[5]    = {nullptr};     // Pointer to the Modbus link object
   ModbusDeviceInfo_struct *_modbus_devices[16] = {nullptr};     // Array of connected Modbus devices

  ModbusLINK_struct *_modbus_link = nullptr;             // Legacy, to be cleaned up
};

static struct ModbusMGR_struct ModbusMGR_singleton = {}; // Singleton instance of the Modbus Manager

#endif // FEAURE_MODBUS
#endif // HELPERS_MODBUS_MGR_H
