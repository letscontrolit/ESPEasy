#include "../PluginStructs/P183_data_struct.h"

#ifdef USES_P183

// #######################################################################################################
// ############## Data structure for plugin 183: Modbus RTU generic sensor interface       ###############
// #######################################################################################################
////# define P183_DEBUG
# ifndef PLUGIN_BUILD_MAX_ESP32
#  undef P183_DEBUG // Debugging switched off
# endif

// Actions for PLUGIN_TASKTIMER_IN event to distinguish between regular read results and scan sequences
# define ACTION_DUMP_RANGE 0xFFFF
# define ACTION_SCAN_BUS   0xFFFE
# define ACTION_READ_CACHE 0xFFFD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor of the plugin data structure. Initializes the data members to default values.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
P183_data_struct::P183_data_struct(struct EventStruct *event) {
  _taskIndex = event->TaskIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the plugin data structure. Clean up any resources used by the plugin.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
P183_data_struct::~P183_data_struct() {
  delete _modbusDevice;
  _modbusDevice = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization. Takes the Modbus device address and link ID as parameters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_init(uint8_t slaveAddress, int linkId)
{
  // Create a fresh Modbus_device object to handle the Modbus communication
  if (_modbusDevice != nullptr) {
    delete _modbusDevice;
    _modbusDevice = nullptr;
  }
  _modbusDevice = new (std::nothrow) ModbusDEVICE_struct();

  if (_modbusDevice == nullptr) {
    # ifndef LIMIT_BUILD_SIZE
    addLogMove(LOG_LEVEL_ERROR, F("P183: Unable to allocate Modbus device object"));
    # endif // LIMIT_BUILD_SIZE
    return false;
  }

  if (!_modbusDevice->init(slaveAddress, linkId, _taskIndex)) {
    return false;
  }
  _modbusDevice->setModbusTimeout(P183_MODBUS_TIMEOUT);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::plugin_exit()
{
  if (_modbusDevice != nullptr) {
    delete _modbusDevice;
    _modbusDevice = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin read function. Queues a new request to read the Modbus registers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_read(struct EventStruct *event) {
  if (_modbusDevice == nullptr) {
    return false;
  }

  for (int outputIndex = 0; outputIndex < P183_NR_OUTPUTS; ++outputIndex)
  {
    // Queue a read request for each active output value. The result will be processed in the task timer event.
    // Use the output index as the event index to identify which output value the result belongs to.
    _modbusDevice->readHoldingRegister(P183_ADDRESS(outputIndex), outputIndex);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handles the PLUGIN_TASKTIMER_IN event.
// This is used to process the results of Modbus read requests and to trigger the next step in a Modbus scan sequence.
// event->idx is used to identify which transaction the result belongs to.
// event->Par1 is used to indicate whether the Modbus read was successful (true) or not (false).
// event->Par2 is used to pass the value read from Modbus when the read was successful.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_task_timer(EventStruct *event)
{
  # ifdef P183_DEBUG
  addLogMove(LOG_LEVEL_INFO,
             strformat(F("P183: TaskTimer called IDX=%d, par1=%d, par2=%d, par3=%d, par4=%d"),
                       event->idx, event->Par1, event->Par2, event->Par3, event->Par4));
  # endif // P183_DEBUG

  if (event->idx == ACTION_DUMP_RANGE) {
    if (event->Par1) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("** Address %u (0x%02X) = %u (0x%02X)"), _lastAddress, _lastAddress, event->Par2, event->Par2));
    } else {
      addLogMove(LOG_LEVEL_INFO, strformat(F("** Address %u (0x%02X) no response"), _lastAddress, _lastAddress));
    }
    _lastAddress++;
    scan_next_address();
    return true;
  }
  else if (event->idx == ACTION_SCAN_BUS) {
    if (event->Par1) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("** Device found at address %u (0x%02X)"), _lastAddress, _lastAddress));
    }
    _lastAddress++;
    scan_next_module();
    return true;
  }
  else if (event->idx == ACTION_READ_CACHE) {
    // This is the result of the regular cache read triggered in plugin_once_per_second. Update the user variables with the cache values.

    if (event->Data == nullptr) {
      addLogMove(LOG_LEVEL_ERROR, F("P183: No data received for cache read"));
      return false;
    }
    ModbusRegisterSet_struct *registerSet = reinterpret_cast<ModbusRegisterSet_struct *>(event->Data);
    int count                             = registerSet->size;

    if (count > _cacheSize) {
      count = _cacheSize; // Prevent overflow if the device returns more registers than the cache can hold
    }

    for (int i = 0; i < count; i++) {
      _RegisterCache[i] = registerSet->data[i]; // Copy the received values to the cache
    }
    return true;
  }
  else  {
    int outputIndex = event->idx;

    if ((outputIndex < 0) || (outputIndex >= P183_NR_OUTPUTS)) {
      # ifndef LIMIT_BUILD_SIZE
      addLogMove(LOG_LEVEL_ERROR, F("P183: Invalid output index in task timer event"));
      # endif // LIMIT_BUILD_SIZE
      return false;
    }

    if (event->Par1) {
      UserVar.setFloat(event->TaskIndex, outputIndex, event->Par2); // Update the user variable with the value read from Modbus
      return true;
    }
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triggered once per second. Fetch the cache values
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P183_data_struct::plugin_once_per_second(EventStruct *event)
{
  if (_modbusDevice == nullptr) {
    return false;
  }

  if (P183_CACHE_SIZE == 0) {
    return true; // Cache not used, nothing to do
  }

  if (P183_CACHE_SIZE > P183_CACHE_SIZE_MAX) {
    return false; // Cache size exceeds maximum allowed size
  }

  // Queue a read request for the cache values. The result will be processed in the task timer event.
  _cacheStart = P183_CACHE_START;
  _cacheSize  = P183_CACHE_SIZE;
  _modbusDevice->readHoldingRegisters(_cacheStart, _cacheSize, ACTION_READ_CACHE);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start iterating over a register range of a Modbus device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::scan_device(uint8_t node_id, uint16_t start_reg, uint16_t end_reg)
{
  uint16_t value = 0;

  addLogMove(LOG_LEVEL_INFO, F("Modbus: dumping module registers"));

  if (_modbusDevice == nullptr) {
    return;
  }

  if (start_reg < end_reg) {
    _lastAddress = start_reg;
    _endAddress  = end_reg;
    _scanning    = true;
    scan_next_address();
  }
  else
  {
    _scanning = false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read the next holding register from the Modbus device
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::scan_next_address()
{
  if (_scanning) {
    if (_lastAddress <= _endAddress) {
      _modbusDevice->readHoldingRegister(_lastAddress, ACTION_DUMP_RANGE);
    } else {
      _scanning = false;
      addLogMove(LOG_LEVEL_INFO, F("Modbus: Finished scanning device"));
    }
    return;
  }
  _scanning = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scan Modbus addreses on the bus and log the results
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::scan_modbus()
{
  addLogMove(LOG_LEVEL_INFO, F("Modbus: Scanning for Modbus modules"));

  if (_modbusDevice == nullptr) {
    return;
  }

  _lastAddress = 1;
  _endAddress  = 247;
  _scanning    = true;
  scan_next_module();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::scan_next_module()
{
  if (_scanning) {
    if (_lastAddress <= _endAddress) {
      _modbusDevice->readModuleHoldingRegister(_lastAddress, 1, ACTION_SCAN_BUS);
    }
    else
    {
      _scanning = false;
      addLogMove(LOG_LEVEL_INFO, F("Modbus: Finished scanning for modules"));
    }
  }
}

#if P183_ALLOW_MODBUS_WAIT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read a Modbus register from the device. Block until the data is available
// Warning: this may take time as we wait for the  Modbus message to be exchanged
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t P183_data_struct::readRegisterWait(uint16_t address) {
  uint16_t value          = 0;
  uint32_t startTime      = millis();
  ModbusResultState state = ModbusResultState::Busy;

  if (_modbusDevice == nullptr) {
    return 0;
  }

  _modbusDevice->readHoldingRegister(address, &value, &state); // Queue the read action

  while (state == ModbusResultState::Busy) {
    delay(50);
    _modbusDevice->processCommand();    // Trigger Modbus facilities to process the Modbus queue

    if (timePassedSince(startTime) > P183_MODBUS_TIMEOUT) {
      state = ModbusResultState::Error; // Timeout, exit the loop with an error state
    }
  }

  return value;
}
#endif // ifdef P183_ALLOW_MODBUS_WAIT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t P183_data_struct::readRegisterCache(uint16_t address)
{
  if ((address < _cacheStart) || (address >= _cacheStart + _cacheSize)) {
    return 0;
  }
  else {
    return _RegisterCache[address - _cacheStart];
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P183_data_struct::writeRegister(uint16_t address, uint16_t value)
{
  if (_modbusDevice == nullptr) {
    return;
  }

  _modbusDevice->writeSingleRegister(address, value, &_lastActionState); // Queue the action (and for now forget it)
}

#endif // ifdef USES_P183
