#ifndef HELPERS_MODBUS_MGR_H
#define HELPERS_MODBUS_MGR_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "../Helpers/Modbus_link.h"
# include "../Helpers/_ESPEasy_key_value_store.h"


// ModbusMGR structure representing the singleton Modbus Management entity
// Thw manager has an overview of all Modbus links and the conneted devices.
// The manager allows multiple Modbus devices to connect to a single Modbus link while supporting multiple links.
// The modbus manager is not involved in the actual data transport, this is handled by a direct relation between Modbus device and
// ModbusLINK object.
typedef struct ModbusMGR_struct  {
  ModbusMGR_struct();
  ~ModbusMGR_struct();

  bool initialize();

  bool connect(int                 linkId,
               ModbusLINK_struct **link,
               uint8_t            *deviceID);

  bool disconnect(uint8_t deviceID);

  void processLinks();

  void dumpAdminInfo();

  void show_modbus_interfaces();
  bool save_modbus_interfaces(String& error);

  bool isInitialized() const { return _initialized; }

private:

  static const int MAX_MODBUS_LINKS   = 4;  // Maximum number of Modbus links supported

  // Structure representing the information of a Modbus link as managed by the Modbus_mgr
  // Includes associated ModbusLINK object, link configuration and config storage
  struct ModbusLinkInfo_struct {
    ESPEasySerialPort         port             = ESPEasySerialPort::not_set;
    int8_t                    serial_rx        = -1;
    int8_t                    serial_tx        = -1;
    int16_t                   baudrate         = 9600;
    int8_t                    dere_pin         = -1;      // Pin used for RS485 DE/RE control, -1 if not used
    bool                      rs485_mode       = false;   // True if RS485 mode is enabled
    bool                      collision_detect = false;   // True if collision detection is enabled
    struct ModbusLINK_struct *link             = nullptr; // Pointer to the Modbus link object
    ESPEasy_key_value_store  *kvs              = nullptr; // Key-value store for storing link-specific settings and parameters

  };

  ModbusLinkInfo_struct _modbus_links[MAX_MODBUS_LINKS] = {};    // All links managed by the Modbus manager
  bool                  _initialized                    = false; // Flag indicating if the manager is initialized
  int                   _deviceCounter                  = 0;

  bool setLink(const int               linkIndex,
               const ESPEasySerialPort port,
               const int8_t            serial_rx,
               const int8_t            serial_tx,
               uint16_t                baudrate,
               int8_t                  dere_pin,
               bool                    collision_detect);

} ModbusMGR_struct_t;

extern ModbusMGR_struct_t ModbusMGR_singleton; // Singleton instance of the Modbus Manager

#endif // FEAURE_MODBUS
#endif // HELPERS_MODBUS_MGR_H
