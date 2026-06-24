#ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
#define PLUGINSTRUCTS_P183_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P183

///# define P183_DEBUG // Switch on additional debug logging
# ifdef BUILD_NO_DEBUG
#  undef P183_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

////# define P183_ALLOW_MODBUS_WAIT 

# include "../Helpers/Modbus_device.h"

// Plugin configuration parameters
// PCONFIG(0) is the Modbus device ID.
// PCONFIG(1) is the Modbus link ID.
// PCONFIG(2) is used for flags for future use. Currently not used.
// PCONFIG(3) is the number of active output values (1-4)
// PCONFIG(4) is the Modbus register address for value 1
// PCONFIG(5) is the Modbus register address for value 2
// PCONFIG(6) is the Modbus register address for value 3
// PCONFIG(7) is the Modbus register address for value 4
// Use P183_ADDRESS(x) to access the PCONFIG value for value x
// PCONFIG_LONG(0) is the number of Modbus registers to keep in cache
// PCONFIG_LONG(1) is the starting register address for the cache
# define P183_DEV_ID           PCONFIG(0)
# define P183_DEV_ID_LABEL     PCONFIG_LABEL(0)
# define P183_LINK_ID          PCONFIG(1)
# define P183_LINK_ID_LABEL    PCONFIG_LABEL(1)
# define P183_NR_OUTPUTS       PCONFIG(3)
# define P183_NR_OUTPUTS_LABEL PCONFIG_LABEL(3)
# define P183_ADDRESS(x) PCONFIG(4 + x)
# define P183_ADDRESS_LABEL(x) concat(F("addr"), x)

# define P183_CACHE_SIZE PCONFIG_LONG(0)
# define P183_CACHE_SIZE_LABEL F("P183sz")
# define P183_CACHE_START PCONFIG_LONG(1)
# define P183_CACHE_START_LABEL F("P183st")

# define P183_DEV_ID_DFLT      1
# define P183_MODBUS_TIMEOUT   1000    // milliseconds
# define P183_MAX_MODBUS_NODES 247
# define P183_MODBUS_BROADCAST_ID 0    // Modbus broadcast address
# define P183_CACHE_SIZE_MAX       100 // Maximum number of registers to keep in cache
# define P183_CACHE_START_MAX 65535    // Maximum starting address for cache

// The default set of single-value VType options
// constexpr uint8_t P183_START_VTYPE = 0;

struct P183_data_struct : public PluginTaskData_base {
  P183_data_struct(struct EventStruct *event);
  P183_data_struct() = delete;
  virtual ~P183_data_struct();

  bool     plugin_init(uint8_t slaveAddress,
                       int     linkId);

  void     plugin_exit();
  bool     plugin_read(struct EventStruct *event);
  bool     plugin_task_timer(struct EventStruct *event);
  bool     plugin_once_per_second(struct EventStruct *event);
  void     scan_device(uint8_t  node_id,
                       uint16_t start_reg,
                       uint16_t end_reg);
  void     scan_modbus();
  #ifdef P183_ALLOW_MODBUS_WAIT
  uint16_t readRegisterWait(uint16_t address);
  #endif // ifdef P183_ALLOW_MODBUS_WAIT
  uint16_t readRegisterCache(uint16_t address);

  void     writeRegister(uint16_t address,
                         uint16_t value);

private:

  taskIndex_t                 _taskIndex                          = INVALID_TASK_INDEX;
  struct ModbusDEVICE_struct *_modbusDevice                       = nullptr;
  uint16_t                    _RegisterCache[P183_CACHE_SIZE_MAX] = { 0 };
  uint16_t                    _cacheStart                         = 0;
  int                         _cacheSize                          = 0;
  ModbusResultState           _lastActionState                    = ModbusResultState::Busy;
  uint16_t                    _lastAddress                        = 0;
  uint16_t                    _endAddress                         = 0;
  bool                        _scanning                           = false;

  void scan_next_address();
  void scan_next_module();

};

#endif // ifdef USES_P183
#endif // ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
