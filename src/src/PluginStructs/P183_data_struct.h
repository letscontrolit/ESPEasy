#ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
#define PLUGINSTRUCTS_P183_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P183

# include "../Helpers/Modbus_device.h"

# define P183_MODBUS_TIMEOUT   1000 // milliseconds

// The default set of single-value VType options
constexpr uint8_t P183_START_VTYPE = 0;

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
  void     writeResgister(uint16_t address,
                          uint16_t value);

private:

  taskIndex_t                 _taskIndex         = INVALID_TASK_INDEX;
  struct ModbusDEVICE_struct *_modbusDevice      = nullptr;
  uint16_t                    _registerValues[4] = {}; // Modus register values retrieved for output values
  ModbusResultState_t         _queueStates[4]    = {}; // State of read hloding register transactions
};

#endif // ifdef USES_P183
#endif // ifndef PLUGINSTRUCTS_P183_DATA_STRUCT_H
