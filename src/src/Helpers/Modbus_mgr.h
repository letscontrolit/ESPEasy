#ifndef HELPERS_MODBUS_MGR_H
#define HELPERS_MODBUS_MGR_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "Modbus_link.h"


// ModbusMGR structure representing the singleton Modbus Management entity
// Thw manager has an overview of all Modbus links and the conneted devices.
// The manager allows multiple Modbus devices to connect to a single Modbus link while supporting multiple links.
// The modbus manager is not involved in the actual data transport, this is handled by a direct relation between Modbus device and
// ModbusLINK object.
typedef struct ModbusMGR_struct  {
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

  void dumpAdminInfo();

private:

  static const int MAX_MODBUS_LINKS   = 5;  // Maximum number of Modbus links supported
  static const int MAX_MODBUS_DEVICES = 16; // Maximum number of Modbus devices supported

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
    uint8_t                       deviceID = 0;                               // Unique ID assigned by the Modbus manager
    struct ModbusDEVICE_struct   *device   = nullptr;                         // Pointer to the Modbus device object
    struct ModbusLinkInfo_struct *link     = nullptr;                         // Pointer to the Modbus link info
  };

  ModbusLinkInfo_struct   *_modbus_links[MAX_MODBUS_LINKS]     = { nullptr }; // Pointer to the Modbus link object
  ModbusDeviceInfo_struct *_modbus_devices[MAX_MODBUS_DEVICES] = { nullptr }; // Array of connected Modbus devices
} ModbusMGR_struct_t;

extern ModbusMGR_struct_t ModbusMGR_singleton;                                // Singleton instance of the Modbus Manager

#endif // FEAURE_MODBUS
#endif // HELPERS_MODBUS_MGR_H
