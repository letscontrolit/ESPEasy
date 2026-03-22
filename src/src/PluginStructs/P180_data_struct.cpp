#include "../PluginStructs/P180_data_struct.h"

#ifdef USES_P180
# include <GPIO_Direct_Access.h>
# include "../Globals/RulesCalculate.h"

P180_data_struct::P180_data_struct(struct EventStruct *event) {
  _enPin     = P180_ENABLE_PIN;
  _rstPin    = P180_RST_PIN;
  _showLog   = P180_LOG_DEBUG == 1;
  _taskIndex = event->TaskIndex;

  BusCmd_Handler_I2C*i2cHandler = new (std::nothrow) BusCmd_Handler_I2C(static_cast<uint8_t>(P180_I2C_ADDRESS));

  if (nullptr != i2cHandler) {
    busCmd_Helper = new (std::nothrow) BusCmd_Helper_struct(i2cHandler,
                                                            _taskIndex,
                                                            _enPin,
                                                            _rstPin,
                                                            P180_NR_OUTPUT_VALUES);

    if (nullptr != busCmd_Helper) {
      busCmd_Helper->setLog(_showLog);

      loadStrings(event);

      if (_stringsLoaded) {
        for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
          busCmd_Helper->setBuffer(i, _strings[P180_BUFFER_START_CACHE + i], _strings[P180_BUFFER_START_COMMANDS + i]);
          _strings[P180_BUFFER_START_CACHE + i].clear(); // Clear memory
          _strings[P180_BUFFER_START_COMMANDS + i].clear();
        }
      }
    }
  }
}

P180_data_struct::~P180_data_struct() {
  _strings->clear();
}

bool P180_data_struct::plugin_init(struct EventStruct *event) {
  if (validGpio(_enPin)) {
    pinMode(_enPin, OUTPUT);
    DIRECT_pinWrite(_enPin, HIGH);
  }

  if (validGpio(_rstPin)) {
    pinMode(_rstPin, OUTPUT);
    DIRECT_pinWrite(_rstPin, HIGH);
  }

  loadStrings(event);

  if (nullptr != busCmd_Helper) {
    _initialized = true;

    if (_stringsLoaded && !_strings[P180_BUFFER_ENTRY_INIT].isEmpty()) {
      _initialized = busCmd_Helper->parseAndExecute(BusCmd_CommandSource_e::PluginRead, // To allow logging
                                                    _strings[P180_BUFFER_ENTRY_INIT],
                                                    F("P180 : Device initialization with %d commands."));
    }
  }

  return _initialized;
}

void P180_data_struct::plugin_exit(struct EventStruct *event) {
  loadStrings(event);

  if (_stringsLoaded && !_strings[P180_BUFFER_ENTRY_EXIT].isEmpty() && (nullptr != busCmd_Helper)) {
    busCmd_Helper->parseAndExecute(BusCmd_CommandSource_e::PluginRead, // To allow logging
                                   _strings[P180_BUFFER_ENTRY_EXIT],
                                   F("P180 : Device shutdown/exit with %d commands."));
  }
}

void P180_data_struct::loadStrings(struct EventStruct*event) {
  if (!_stringsLoaded) {
    LoadCustomTaskSettings(event->TaskIndex, _strings, P180_CUSTOM_BUFFER_SIZE, 0);
    _stringsLoaded = true;
  }
}

bool P180_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (nullptr != busCmd_Helper) {
    return busCmd_Helper->plugin_once_a_second(event);
  }
  return false;
}

bool P180_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if (nullptr != busCmd_Helper) {
    return busCmd_Helper->plugin_ten_per_second(event);
  }
  return false;
}

bool P180_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (nullptr != busCmd_Helper) {
    return busCmd_Helper->plugin_fifty_per_second(event);
  }
  return false;
}

bool P180_data_struct::plugin_read(struct EventStruct *event) {
  if (nullptr != busCmd_Helper) {
    return busCmd_Helper->plugin_read(event);
  }
  return false;
}

/*********************************************************************************************
 * Handle command processing
 ********************************************************************************************/
bool P180_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success     = false;
  const String cmd = parseString(string, 1);

  if (equals(cmd, F("geni2c"))) {
    const String sub       = parseString(string, 2);
    const String par3      = parseStringKeepCaseNoTrim(string, 3);
    const String varName   = parseString(string, 4);
    const bool   hasPar3   = !par3.isEmpty();
    const bool   hasPar4   = !varName.isEmpty();
    const bool   hasPar5   = !parseString(string, 5).isEmpty();
    const bool   hasBusCmd = nullptr != busCmd_Helper;
    std::vector<BusCmd_Command_struct> cmds;
    taskVarIndex_t taskVar = INVALID_TASKVAR_INDEX;
    uint32_t val{};

    if (hasPar4) {
      if (validUIntFromString(varName, val) && (val > 0) && (val <= VARS_PER_TASK)) {
        taskVar = val - 1;
      } else {
        taskVar = findDeviceValueIndexByName(varName, event->TaskIndex);
      }
    }

    if (equals(sub, F("cmd")) && hasPar3 && hasBusCmd) { // genI2c,cmd,<I2Ccommands>[,<TaskValueIndex>[,<cache_name>]]
      String cacheName;

      if (hasPar5) {
        cacheName = parseString(string, 5);
      }

      BusCmd_CommandSource_e old = busCmd_Helper->getCommandSource();
      busCmd_Helper->setCommandSource(BusCmd_CommandSource_e::PluginWrite);
      cmds = busCmd_Helper->parseBusCmdCommands(cacheName, par3, !cacheName.isEmpty());
      busCmd_Helper->setCommandSource(old);
    } else if (equals(sub, F("exec")) && hasPar3 && hasBusCmd) {     // genI2c,exec,<cache_name>[,<TaskValueIndex>]
      cmds = busCmd_Helper->parseBusCmdCommands(par3, EMPTY_STRING); // Fetch commands from cache by name
    } else if (equals(sub, F("log")) && hasPar3) {                   // genI2c,log,<1|0>
      _showLog       = event->Par2 != 0;
      P180_LOG_DEBUG = _showLog ? 1 : 0;

      if (hasBusCmd) {
        busCmd_Helper->setLog(_showLog);
      }
      success = true;
    }

    if (!cmds.empty() && hasBusCmd && (BusCmd_CommandState_e::Idle == busCmd_Helper->getCommandState())) { // Execute commands when not busy
      busCmd_Helper->setCommands(cmds,
                                 taskVar,
                                 0,
                                 1, // Process single entry
                                 BusCmd_CommandState_e::Processing);
      Scheduler.schedule_task_device_timer(_taskIndex, millis() + 5);

      success = true;
    }
  }

  return success;
}

/*********************************************************************************************
 * Handle get config value retrieval processing
 ********************************************************************************************/
# ifndef LIMIT_BUILD_SIZE
bool P180_data_struct::plugin_get_config(struct EventStruct *event,
                                         String            & string) {
  bool success         = false;
  const bool hasBusCmd = nullptr != busCmd_Helper;

  if (hasBusCmd) {
    success = busCmd_Helper->plugin_get_config(event, string);
  }

  return success;
}

# endif // ifndef LIMIT_BUILD_SIZE

#endif // ifdef USES_P180
