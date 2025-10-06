#include "_Plugin_Helper.h"
#ifdef USES_P180

// #######################################################################################################
// ################################## Plugin-180: Generic I2C interface ##################################
// #######################################################################################################

/** Changelog:
 * 2025-06-04 tonhuisman: Add [<taskname>#log] to Get Config to fetch the current Parsing and execution log setting
 * 2025-06-03 tonhuisman: Restore PLUGIN_GET_CONFIG_VALUE support, to allow I2C operations to be executed and return a value without
 *                        defining extra I2C Command definitions. Not available for LIMIT_BUILD_SIZE builds
 * 2025-05-13 tonhuisman: Add support for String (str) data-format handling
 * 2025-05-12 tonhuisman: Refactor I2C handling into replaceble BusCmd_Handler interface & I2C implemented handler
 * 2025-05-10 tonhuisman: Refactor I2C Command processor to BusCmd command processor helper (phase 1 refactor)
 * 2025-05-05 tonhuisman: Change abbreviation for 'enable' from 'l' to 'a'
 *                        Add 'let.<var>.<value>' I2C command, abbreviated to 'l'. Assigns <value> to a Rules global variable
 *                        Limit max delay value to 10msec for 1/sec, 10/sec and 50/sec processing
 * 2025-05-02 tonhuisman: Respond successful to PLUGIN_I2C_HAS_ADDRESS request, as we can (probably) handle any I2C device...
 * 2025-05-01 tonhuisman: Suppress logging during handling of 1/sec, 10/sec and 50/sec events for better performance
 *                        Add genI2C,log,<0|1> subcommand for disabling/enabling plugin logging
 *                        Remove support for PLUGIN_GET_CONFIG as its not needed or useable and not implemented yet
 * 2025-04-28 tonhuisman: Add support for processing during 1/sec, 10/sec and 50/sec plugin events
 * 2025-04-27 tonhuisman: Add support for executing a command sequence from cache: getI2C,exec,<cache-name>[,<TaskVarIndex>]
 *                        Add support for selecting the Value-index either by number or name
 * 2025-04-21 tonhuisman: Add preliminary support for MQTT Discovery
 * 2025-04-13 tonhuisman: Add 'if' command, some optimizations
 * 2025-04-10 tonhuisman: First version made available. Most plugin features implemented, few todos to resolve.
 * 2025-03-27 tonhuisman: Start plugin development
 */

/** I2C Commands supported:
 * short : long = Description
 * n : nop = No Operation
 * g : get = Read
 * p : put = Write
 * r : read = Read from a register
 * w : write = Write to a register
 * s : read16 = Read from a 16 bit register (to be implemented per BusCmd_Handler implementation)
 * t : write16 = Write to a 16 bit register (to be implemented per BusCmd_Handler implementation)
 * e : eval = Set previous command data to eval for calc and/or if commands
 * c : calc = Calculate a result from eval data retrieved
 * i : if = Calculate a result from eval data and cancel execution if the result is 0
 * v : value = Put current result in Value
 * d : delay = Delay msec. until next command is executed (asynchronous/non-blocking when > 10 msec)
 * a : enable = Set Enable GPIO pin to a state
 * z : reset = Pulse Reset GPIO pin to a state for n msec.
 * l : let = Assign value to global variable, usable in calculations (and Rules)
 *
 * Command structure: (period . for argument separator, semicolon ; for command separator, | for event separator)
 * <cmd>[.<fmt>[.<len>]][.<register>][.<data>];...[|(1ps|10ps|50ps)|<cmd_sequences>]
 *
 * get.<fmt>[.<len>]
 * put.<fmt>.<data>[.<data>...]
 * read.<fmt>[.<len>].<reg>
 * write.<fmt>.<reg>.<data>[.<data>...]
 * eval
 * value.<valueIndex> (1..4)
 * calc.<calculation> Like Rules, extra available vars: %value%, %pvalue%, %h%, %b0%..%b<n>%, %bx0%..%bx<n>%, %w0%..%w<n>%, %wx0%..%wx<n>%
 * if.<calculation> Similar to 'calc', when the calculation-result is 0 execution is cancelled
 * delay.<msec> (range: 0..500)
 * enable.<state> (0 or 1)
 * reset.<state>.<msec> (state = 0 or 1, msec = range: 0..500)
 * let.<var>.<calculation> Similar to 'calc', result is stored in global <var>, can be used like [var#<var>], [int#<var>] or %v_<var>%
 *
 * fmt options:
 * u8 : uint8_t (1 byte)
 * u16 : uint16_t (2 bytes)
 * u24 : uint24_t (3 bytes)
 * u32 : uint32_t (4 bytes)
 * u16le : uint16_t (2 bytes, little endian)
 * u24le : uint24_t (3 bytes, little endian)
 * u32le : uint32_t (4 bytes, little endian)
 * 8 : int8_t (1 bytes, signed)
 * 16 : int16_t (2 bytes, signed)
 * 24 : int24_t (3 bytes, signed)
 * 32 : int32_t (4 bytes, signed)
 * 16le : int16_t (2 bytes, signed, little endian)
 * 24le : int24_t (3 bytes, signed, little endian)
 * 32le : int32_t (4 bytes, signed, little endian)
 * b[.<len>] : <len> uint8_t (bytes), <len> is needed for reading only
 * w[.<len>] : <len> uint16_t (words), <len> is needed for reading only
 * str[.<len>] : <len> String, <len> is needed for reading only
 */

/** 1/sec, 10/sec and 50/sec plugin event support:
 * I2C Command sequences can be defined for PLUGIN_ONCE_A_SECOND, PLUGIN_TEN_PER_SECOND and PLUGIN_FIFTY_PER_SECOND by prefixing the
 * command with 1ps, 10ps or 50ps, and a pipe-symbol | separator (the event separator).
 * When a regular PLUGIN_READ event is also used, the previous commands must be postfixed with a pipe too, so that could look like this:
 * <plugin_read_commands>|10ps|<plugin_ten_per_second_command_sequence>
 * instead of 10ps also 1ps or 50ps could be used, though for 50ps that might incure some performance issues
 */

/** Write commands supported:
 * geni2c,cmd,<I2C-commands>[,<TaskVarIndex>[,<cache-name>]] : Parse command sequence (opt. into cache), execute and store value in TaskVar
 * getI2C,exec,<cache-name>[,<TaskVarIndex>] : Execute command sequence from cache and store value in TaskVar
 * getI2C,log,<0|1> : Set the Logging option disabled or enabled. Not saved automatically.
 */

/** Get Config Value support:
 * [<taskname>#log] : Fetch current state of Parsing and execution log, 0 or 1.
 * Use variable like:
 * [<taskname>#<varname>,<I2C-commands>]
 * - <taskname> must match the current task name
 * - <varname> can be any valid value name, except one that is already used by the plugin or 'log'
 * - <I2C-commands> a sequence of I2C commands, as described above. The value command should be avoided, as it will set any of the
 *   values possibly in use, and not return the last value retrieved fron I2C commands.
 *   The resulting value will be returned.
 */

# define PLUGIN_180
# define PLUGIN_ID_180         180
# define PLUGIN_NAME_180       "Generic - I2C Generic"
# define PLUGIN_VALUENAME1_180 "Value1"
# define PLUGIN_VALUENAME2_180 "Value2"
# define PLUGIN_VALUENAME3_180 "Value3"
# define PLUGIN_VALUENAME4_180 "Value4"

# include "src/PluginStructs/P180_data_struct.h"

boolean Plugin_180(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_180;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.FormulaOption  = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_180);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_180));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_180));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_180));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_180));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(P180_SENSOR_TYPE_INDEX) = static_cast<int16_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P180_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P180_SENSOR_TYPE_INDEX));
      event->idx        = P180_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    # if defined(FEATURE_MQTT_DISCOVER) && FEATURE_MQTT_DISCOVER // When feature is available
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      for (uint8_t i = 0; i < P180_NR_OUTPUT_VALUES; ++i) {
        event->ParN[i] = PCONFIG(P180_VALUE_OFFSET + i); // Selection is limited to single-value VTypes
      }
      success = true;
      break;
    }
    # endif // if defined(FEATURE_MQTT_DISCOVER) && FEATURE_MQTT_DISCOVER

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = true;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P180_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"),
                     formatToHex_decimal(P180_I2C_ADDRESS), 4);

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(PinSelectPurpose::Generic_output, F("'Enable' GPIO"), F("taskdevicepin1"), P180_ENABLE_PIN);
      addFormPinSelect(PinSelectPurpose::Generic_output, F("'Reset' GPIO"),  F("taskdevicepin2"), P180_RST_PIN);

      sensorTypeHelper_webformLoad_allTypes(event, P180_SENSOR_TYPE_INDEX);

      constexpr uint8_t    maxVType = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_NOT_SET);
      std::vector<uint8_t> singleOptions;

      // Build a list of all single-value available value VTypes from PR #5199
      for (uint8_t i = P180_START_VTYPE; i < maxVType; ++i) {
        if (getValueCountFromSensorType(static_cast<Sensor_VType>(i), false) == 1) {
          singleOptions.push_back(i);
        }
      }
      String strings[P180_CUSTOM_BUFFER_SIZE];
      LoadCustomTaskSettings(event->TaskIndex, strings, P180_CUSTOM_BUFFER_SIZE, 0);

      addFormTextBox(F("I2C Init Commands"),
                     getPluginCustomArgName(40),
                     strings[P180_BUFFER_ENTRY_INIT],
                     P180_MAX_COMMAND_LENGTH);
      addUnit(strformat(F("%d of %d used"), strings[P180_BUFFER_ENTRY_INIT].length(), P180_MAX_COMMAND_LENGTH));

      addFormTextBox(F("I2C Exit Commands"),
                     getPluginCustomArgName(41),
                     strings[P180_BUFFER_ENTRY_EXIT],
                     P180_MAX_COMMAND_LENGTH);
      addUnit(strformat(F("%d of %d used"), strings[P180_BUFFER_ENTRY_EXIT].length(), P180_MAX_COMMAND_LENGTH));

      addFormCheckBox(F("Parsing & executing log (INFO)"), F("plog"), P180_LOG_DEBUG == 1);

      for (uint8_t i = 0; i < P180_NR_OUTPUT_VALUES; ++i) {
        addFormSubHeader(strformat(F("Value %d I2C Commands"), i + 1));

        // type
        sensorTypeHelper_webformLoad(event, P180_VALUE_OFFSET + i, singleOptions.size(), &singleOptions[0], false, i + 1);

        // Name
        addFormTextBox(strformat(F("Cache-Name %d (optional)"), i + 1),
                       getPluginCustomArgName(20 + i),
                       strings[P180_BUFFER_START_CACHE + i],
                       P180_MAX_NAME_LENGTH);

        // Add I2C commands sequence
        addFormTextBox(concat(F("I2C Commands "), i + 1),
                       getPluginCustomArgName(30 + i),
                       strings[P180_BUFFER_START_COMMANDS + i],
                       P180_MAX_COMMAND_LENGTH);
        addUnit(strformat(F("%d of %d used"), strings[P180_BUFFER_START_COMMANDS + i].length(), P180_MAX_COMMAND_LENGTH));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const String addr = webArg(F("i2c_addr"));
      P180_I2C_ADDRESS = (uint8_t)strtol(addr.c_str(), 0, 16);
      P180_ENABLE_PIN  = getFormItemInt(F("taskdevicepin1"));
      P180_RST_PIN     = getFormItemInt(F("taskdevicepin2"));
      P180_LOG_DEBUG   = isFormItemChecked(F("plog")) ? 1 : 0;
      pconfig_webformSave(event, P180_SENSOR_TYPE_INDEX);

      for (uint8_t i = 0; i < P180_NR_OUTPUT_VALUES; ++i) {
        PCONFIG(P180_VALUE_OFFSET + i) = getFormItemInt(sensorTypeHelper_webformID(P180_VALUE_OFFSET + i));
      }

      {
        String strings[P180_CUSTOM_BUFFER_SIZE];

        for (uint8_t varNr = 0; varNr < VARS_PER_TASK; varNr++) {
          strings[P180_BUFFER_START_CACHE + varNr]    = webArg(getPluginCustomArgName(20 + varNr)); // Name
          strings[P180_BUFFER_START_COMMANDS + varNr] = webArg(getPluginCustomArgName(30 + varNr)); // Commands
        }
        strings[P180_BUFFER_ENTRY_INIT] = webArg(getPluginCustomArgName(40));                       // INIT Commands
        strings[P180_BUFFER_ENTRY_EXIT] = webArg(getPluginCustomArgName(41));                       // EXIT Commands

        const String error = SaveCustomTaskSettings(event->TaskIndex, strings, P180_CUSTOM_BUFFER_SIZE, 0);

        if (!error.isEmpty()) {
          addHtmlError(error);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P180_I2C_ADDRESS > 0) {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P180_data_struct(event));
        P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P180_data) {
          success = P180_data->plugin_init(event);
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("P180 : I2C address not set."));
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        P180_data->plugin_exit(event);
      }
      break;
    }

    case PLUGIN_READ:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_read(event);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_once_a_second(event);
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_ten_per_second(event);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_fifty_per_second(event);
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_write(event, string);
      }

      break;
    }

    # ifndef LIMIT_BUILD_SIZE
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P180_data_struct *P180_data = static_cast<P180_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P180_data) {
        success = P180_data->plugin_get_config(event, string);
      }

      break;
    }
    # endif // ifndef LIMIT_BUILD_SIZE
  }
  return success;
}

#endif // USES_P180
