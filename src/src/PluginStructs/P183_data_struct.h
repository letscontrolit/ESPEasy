#ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
#define PLUGINSTRUCTS_P183_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P183

# define P183_DEBUG // Switch on additional debug logging
# ifdef BUILD_NO_DEBUG
#  undef P183_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

# include "../Helpers/Modbus_device.h"

// Plugin configuration parameters
// PCONFIG(0) is the Modbus device ID.
// PCONFIG(1) is the serial baud rate.
// PCONFIG(2) is used for flags, where bit 0 indicates collision detection
// PCONFIG(3) is the number of active output values (1-4)
// PCONFIG(4) is the Modbus register address for value 1
// PCONFIG(5) is the Modbus register address for value 2
// PCONFIG(6) is the Modbus register address for value 3
// PCONFIG(7) is the Modbus register address for value 4
// Use P183_ADDRESS(x) to access the PCONFIG value for value x
# define P183_DEV_ID           PCONFIG(0)
# define P183_DEV_ID_LABEL     PCONFIG_LABEL(0)
# define P183_BAUDRATE         PCONFIG(1)
# define P183_BAUDRATE_LABEL   PCONFIG_LABEL(1)
# define P183_NR_OUTPUTS       PCONFIG(3)
# define P183_NR_OUTPUTS_LABEL PCONFIG_LABEL(3)
# define P183_ADDRESS(x) PCONFIG(4 + x)
# define P183_ADDRESS_LABEL(x) concat(F("addr"), x)

# define P183_GET_FLAG_COLL_DETECT bitRead(PCONFIG(2), 0)
# define P183_SET_FLAG_COLL_DETECT(x) bitWrite(PCONFIG(2), 0, x)
# define P183_FLAG_COLL_DETECT_LABEL "colldet"

# define P183_DEPIN           CONFIG_PIN3

# define P183_DEV_ID_DFLT      1
# define P183_BAUDRATE_DFLT    3    // 9600 baud
# define P183_MAX_BAUDRATE_SEL 8
# define P183_MODBUS_TIMEOUT   1000 // milliseconds
# define P183_MAX_MODBUS_NODES 247
# define P183_MODBUS_BROADCAST_ID 0 // Modbus broadcast address

// The default set of single-value VType options
// constexpr uint8_t P183_START_VTYPE = 0;

struct P183_data_struct : public PluginTaskData_base {
  P183_data_struct(struct EventStruct *event);
  P183_data_struct() = delete;
  virtual ~P183_data_struct();

  bool plugin_init(uint8_t                 slaveAddress,
                   const ESPEasySerialPort port,
                   const int16_t           serial_rx,
                   const int16_t           serial_tx,
                   int16_t                 baudrate,
                   int8_t                  dere_pin,
                   bool                    collision_detect);

  void     plugin_exit();
  bool     plugin_read(struct EventStruct *event);
  bool     plugin_once_a_second(struct EventStruct *event);
  bool     plugin_ten_per_second(struct EventStruct *event);
  void     scan_device(uint8_t node_id,
                       uint8_t start_reg,
                       uint8_t end_reg);
  void     scan_modbus();
  uint16_t readRegisterWait(uint16_t address);
  void     writeRegister(uint16_t address,
                         uint16_t value);

private:

  taskIndex_t                 _taskIndex         = INVALID_TASK_INDEX;
  struct ModbusDEVICE_struct *_modbusDevice      = nullptr;
  uint16_t                    _registerValues[4] = {}; // Modus register values retrieved for output values
  ModbusResultState           _queueStates[4]    = {}; // State of read hloding register transactions
  ModbusResultState           _lastActionState   = ModbusResultState::Busy;
};

#endif // ifdef USES_P183
#endif // ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
