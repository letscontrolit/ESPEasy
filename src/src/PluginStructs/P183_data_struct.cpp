#include "P183_data_struct.h"

#ifdef USES_P183

// #######################################################################################################
// ############## Data structure for plugin 183: Modbus RTU generic sensor interface       ###############
// #######################################################################################################

# define P183_NR_OUTPUTS       PCONFIG(3)
# define P183_ADDRESS(x) PCONFIG(4 + x)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
P183_data_struct::P183_data_struct(struct EventStruct *event) {
  _taskIndex = event->TaskIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
P183_data_struct::~P183_data_struct() {
  plugin_exit();  // Destruct dynamic structures contained in this object
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_init(uint8_t                 slaveAddress,
                                   const ESPEasySerialPort port,
                                   const int16_t           serial_rx,
                                   const int16_t           serial_tx,
                                   int16_t                 baudrate,
                                   int8_t                  dere_pin,
                                   bool                    collision_detect) {
  // Create a fresh Modbus_device object to handle the Modbus communication
  if (_modbusDevice != nullptr) {
    delete _modbusDevice;
    _modbusDevice = nullptr;
  }
  _modbusDevice = new (std::nothrow) ModbusDEVICE_struct();

  if (_modbusDevice == nullptr) {
    addLogMove(LOG_LEVEL_ERROR, F("P183: Unable to allocate Modbus device object"));
    return false;
  }

  // Initialize our own Modbus_device with the provided serial link parameters
  // Note that the link configuration is expected to be the same for all plugins reusing the same serial port
  if (!_modbusDevice->init(slaveAddress, port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
    return false;
  }
  _modbusDevice->setModbusTimeout(P183_MODBUS_TIMEOUT);

  # ifdef P183_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("P183: Init serial: RX pin ");
    log += CONFIG_PIN1;
    log += F(", TX pin ");
    log += CONFIG_PIN2;
    log += F(", RS485 mode selected on pin ");
    log += P183_DEPIN;
    log += F(", baudrate ");
    log += P183_storageValueToBaudrate(P183_BAUDRATE);
    log += F(", collision detection ");
    log += P183_GET_FLAG_COLL_DETECT ? F("enabled") : F("disabled");
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef P183_DEBUG


  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::plugin_exit()
{
  if (_modbusDevice == nullptr) {
    delete _modbusDevice;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_once_a_second(struct EventStruct *event) {
  // TODO
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if (nullptr != _modbusDevice) {
    _modbusDevice->processCommand();
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin read function implementataion.
// A new request to update the Modbus registers is queued. The plugin output valuea are updated with the previous results
bool P183_data_struct::plugin_read(struct EventStruct *event) {
  if (_modbusDevice == nullptr) {
    return false;
  }

  for (int outputIndex = 0; outputIndex < P183_NR_OUTPUTS; ++outputIndex)
  {
    _modbusDevice->readHoldingRegister(P183_ADDRESS(outputIndex), &(_registerValues[outputIndex]), &(_queueStates[outputIndex]));
    UserVar.setFloat(event->TaskIndex, outputIndex, _registerValues[outputIndex]);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::scan_device(uint8_t node_id, uint8_t start_reg, uint8_t end_reg)
{
  String   log;
  uint16_t value           = 0;
  ModbusQueueState_t state = ModbusQueueState::NOT_QUEUED;

  addLogMove(LOG_LEVEL_INFO, F("Modbus: dumping module registers"));

  //  for (uint8_t reg = start_reg; reg <= end_reg; reg++) {
  //    int result = _modbusDevice->readHoldingRegister(reg, &value, &state);
  //    // TODO: Find a way to manage the delay on the queue
  //    log += F("** Address ");
  //    log += String(reg);
  //    log += F(" (0x");
  //    log += String(reg, HEX);
  //
  //    if (result == 0) {
  //      log += F(") = ");
  //      log += String(value);
  //    } else {
  //      log += F(") invalid");
  //    }
  //    addLogMove(LOG_LEVEL_INFO, log);
  //  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scan Modbus addreses from 0x00 to 0xFF for a given node ID
void P183_data_struct::scan_modbus()
{
  String   log;
  uint16_t value           = 0;
  ModbusQueueState_t state = ModbusQueueState::NOT_QUEUED;

  addLogMove(LOG_LEVEL_INFO, F("Modbus: Scanning for Modbus modules"));

  //  for (uint8_t id = 0; id <= 247; id++) {
  //    // TODO: how to scan the Modbus devices in teh new structure
  //    int result = P183_modbus_readRegister(1, &value);
  //    log += F("** Address ");
  //    log += String(id);
  //
  //    if (result == 0) {
  //      log += F(" OK");
  //    } else {
  //      log += F(" no response");
  //    }
  //    addLogMove(LOG_LEVEL_INFO, log);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read a Modbus register from the device. Wait untial the data is available
// Warning: this may take time as we waith for the  Modbus message to be exchanged
uint16_t P183_data_struct::readRegisterWait(uint16_t address) {
  uint16_t value            = 0;
  ModbusResultState_t state = ModbusResultState::BUSY;

  if (_modbusDevice == nullptr) {
    return 0;
  }

  _modbusDevice->readHoldingRegister(address, &value, &state); // Queue the read action

  while (state == ModbusResultState::BUSY) {
    _modbusDevice->processCommand();                           // Trigger Modbus facilities to process the Modbus queue
  }
  return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::writeResgister(uint16_t address, uint16_t value)
{
  static ModbusResultState_t state = ModbusResultState::BUSY; // Static ias it will be accessed after teh function has finished

  if (_modbusDevice == nullptr) {
    return;
  }

  _modbusDevice->writeSingleRegister(address, value, &state); // Queue the action (and for now forget it)
}

#endif // ifdef USES_P183
