#include "_Plugin_Helper.h"

#ifdef USES_P124

// #######################################################################################################
// ########################### Plugin 124 I2C Multi Relay module      ###############################
// #######################################################################################################

/** Changelog:
 * 2021-11-21 tonhuisman: Implement configurable I2C addresses, limited to 0x11..-x18 range (8 units) though
 *                        the boards support any I2C address from 0x00 to 0x7F
 *                        Add Relay state on exit/disabling of the plugin.
 *                        WARNING: Is also triggered when the task is enabled and the settings are saved!
 * 2021-11-19 tonhuisman: Add protection for single initialization of relays (per reset/boot/powercycle)
 * 2021-11-18 tonhuisman: Implement settings,
 *                        Implement write commands:
 *                        multirelay,on,<channel> (channel 1..8, max. accepted as configured)
 *                        multirelay,off,<channel>
 *                        multirelay,set,<bit-relay-8..1> (value in decimal, 0xnn hex or 0bnnnnnnnn binary)
 *                        Add binary state to Devices screen, when plugin enabled.
 * 2021-11-17 tonhuisman: Initial plugin development.
 */

# define PLUGIN_124
# define PLUGIN_ID_124          124
# define PLUGIN_NAME_124        "Output - I2C Multi Relay"
# define PLUGIN_VALUENAME1_124  "State"
# define PLUGIN_VALUENAME2_124  "Channel"
# define PLUGIN_VALUENAME3_124  "Get"

# include "./src/PluginStructs/P124_data_struct.h"

static uint32_t P124_InitializedRelays = 0u;

boolean Plugin_124(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_124;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_124);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_124));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_124));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_124));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P124_CONFIG_I2C_ADDRESS                      = 0x11; // Default I2C address
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;    // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;    // No decimals needed
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0;    // No decimals needed
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // This range of I2C addresses is also maintained in the constructor of P124_data_struct, and must stay consecutive!
      const uint8_t i2cAddressValues[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2caddress"), 8, i2cAddressValues, P124_CONFIG_I2C_ADDRESS);

        addFormCheckBox(F("Change I2C address of board"), F("change_i2c"), false);
        addFormNote(
          F("Change of address will be stored in the board and retained until changed again. See documentation for change-procedure."));
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P124_CONFIG_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *optionsMode2[] = {
        F("2"),
        F("4"),
        F("8") };
      const int optionValuesMode2[] { 2, 4, 8 };
      addFormSelector(F("Number of relays"), F("relays"), 3, optionsMode2, optionValuesMode2, P124_CONFIG_RELAY_COUNT, true);

      addFormSelector_YesNo(F("Initialize relays on startup"),
                            getPluginCustomArgName(P124_FLAGS_INIT_RELAYS),
                            bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_RELAYS) ? 1 : 0, true);
      String label;

      if (bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_RELAYS)) {
        addFormCheckBox(F("Apply initial state always"),
                        getPluginCustomArgName(P124_FLAGS_INIT_ALWAYS),
                        bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_ALWAYS));
        addFormNote(F("Disabled: Applied once per restart, Enabled: Applied on every plugin start, like on Submit of this page"));

        for (int i = 0; i < P124_CONFIG_RELAY_COUNT; i++) {
          label  = F("Relay ");
          label += i + 1;
          label += F(" initial state (on/off)");
          addFormCheckBox(label, getPluginCustomArgName(i), bitRead(P124_CONFIG_FLAGS, i));
        }
      }

      addFormSelector_YesNo(F("Reset relays on exit"),
                            getPluginCustomArgName(P124_FLAGS_EXIT_RELAYS),
                            bitRead(P124_CONFIG_FLAGS, P124_FLAGS_EXIT_RELAYS) ? 1 : 0, true);

      if (bitRead(P124_CONFIG_FLAGS, P124_FLAGS_EXIT_RELAYS)) {
        for (int i = 0; i < P124_CONFIG_RELAY_COUNT; i++) {
          label  = F("Relay ");
          label += i + 1;
          label += F(" exit-state (on/off)");
          addFormCheckBox(label, getPluginCustomArgName(i + P124_FLAGS_EXIT_OFFSET), bitRead(P124_CONFIG_FLAGS, i + P124_FLAGS_EXIT_OFFSET));
        }
        addFormNote(F("ATTENTION: These Relay states will be set when the task is enabled and the settings are saved!"));
      }

      addFormCheckBox(F("Loop Channel/Get on read"),
                      getPluginCustomArgName(P124_FLAGS_LOOP_GET),
                      bitRead(P124_CONFIG_FLAGS, P124_FLAGS_LOOP_GET));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P124_CONFIG_RELAY_COUNT = getFormItemInt(F("relays"));
      P124_CONFIG_I2C_ADDRESS = getFormItemInt(F("i2caddress"));
      uint32_t lSettings = 0u;
      bitWrite(lSettings, P124_FLAGS_INIT_RELAYS, getFormItemInt(getPluginCustomArgName(P124_FLAGS_INIT_RELAYS)) == 1);
      bitWrite(lSettings, P124_FLAGS_INIT_ALWAYS, isFormItemChecked(getPluginCustomArgName(P124_FLAGS_INIT_ALWAYS)));
      bitWrite(lSettings, P124_FLAGS_EXIT_RELAYS, getFormItemInt(getPluginCustomArgName(P124_FLAGS_EXIT_RELAYS)) == 1);
      bitWrite(lSettings, P124_FLAGS_LOOP_GET,    isFormItemChecked(getPluginCustomArgName(P124_FLAGS_LOOP_GET)));

      if (lSettings != 0) {
        for (int i = 0; i < P124_CONFIG_RELAY_COUNT; i++) { // INIT and EXIT states
          bitWrite(lSettings, i,                          isFormItemChecked(getPluginCustomArgName(i)));
          bitWrite(lSettings, i + P124_FLAGS_EXIT_OFFSET, isFormItemChecked(getPluginCustomArgName(i + P124_FLAGS_EXIT_OFFSET)));
        }
      }
      P124_CONFIG_FLAGS = lSettings;

      if (isFormItemChecked(F("change_i2c"))) {
        P124_data_struct *P124_data = new (std::nothrow) P124_data_struct(P124_CONFIG_I2C_ADDRESS, P124_CONFIG_RELAY_COUNT, true);

        if (nullptr != P124_data) {
          P124_data->init();
          P124_CONFIG_I2C_ADDRESS++; // Increment, like the Change Address argument does.

          if (P124_CONFIG_I2C_ADDRESS > 0x18) { P124_CONFIG_I2C_ADDRESS = 0x11; }

          delete P124_data;
          P124_data = nullptr;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P124_data_struct(P124_CONFIG_I2C_ADDRESS, P124_CONFIG_RELAY_COUNT));
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P124_data) && P124_data->init()) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO,
                     concat(F("MultiRelay: Initialized, firmware version: "), static_cast<int>(P124_data->getFirmwareVersion())));
        }

        if (bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_RELAYS) &&
            (!bitRead(P124_InitializedRelays, event->TaskIndex) ||
             bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_ALWAYS))) {
          P124_data->channelCtrl(get8BitFromUL(P124_CONFIG_FLAGS, P124_FLAGS_INIT_OFFSET)); // Set relays state
          UserVar[event->BaseVarIndex] = P124_data->getChannelState();                      // Get relays state
          bitSet(P124_InitializedRelays, event->TaskIndex);                                 // Update initialization status
        }
        P124_data->setLoopState(bitRead(P124_CONFIG_FLAGS, P124_FLAGS_LOOP_GET));           // Loop state
        success = true;
      } else {
        addLog(LOG_LEVEL_ERROR, F("MultiRelay: Initialization error!"));
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P124_data) {
        if (P124_data->isInitialized()) {
          P124_data->channelCtrl(get8BitFromUL(P124_CONFIG_FLAGS, P124_FLAGS_EXIT_OFFSET)); // Set relays state
          UserVar[event->BaseVarIndex] = P124_data->getChannelState();                      // Get relays state
        }
        addLog(LOG_LEVEL_INFO, F("MultiRelay: Object still alive."));
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P124_data) && P124_data->isInitialized()) {
        UserVar[event->BaseVarIndex] = P124_data->getChannelState(); // Get relays state

        if (P124_data->isLoopEnabled()) {
          uint8_t chan = P124_data->getNextLoop();
          uint8_t data = P124_data->getChannelState() & (1 << (chan - 1));
          UserVar[event->BaseVarIndex + 1] = chan;
          UserVar[event->BaseVarIndex + 2] = (data ? 1 : 0);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P124_data) && P124_data->isInitialized()) {
        uint8_t varNr = 3; // VARS_PER_TASK;
        String  label = F("Relay state ");
        label += P124_CONFIG_RELAY_COUNT;
        label += F("..");
        label += 1;
        String   state = F("0b ");
        uint32_t val   = UserVar[event->BaseVarIndex];
        val   &= 0xff;
        val   |= (0x1 << P124_CONFIG_RELAY_COUNT);
        state += ull2String(val, 2);
        state.remove(3, 1); // Delete leading 1 we added
        pluginWebformShowValue(event->TaskIndex, varNr++, label, state, true);

        // success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      # if FEATURE_I2C_DEVICE_CHECK

      if (!I2C_deviceCheck(P124_CONFIG_I2C_ADDRESS, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) {
        break; // Will return the default false for success
      }
      # endif // if FEATURE_I2C_DEVICE_CHECK
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P124_data) {
        return success;
      }

      # ifdef P124_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, string);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Par1..3:");
        log += event->Par1;
        log += ',';
        log += event->Par2;
        log += ',';
        log += event->Par3;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P124_DEBUG_LOG

      String command = parseString(string, 1);

      if (P124_data->isInitialized() &&
          equals(command, F("multirelay"))) {
        String subcommand = parseString(string, 2);

        if (equals(subcommand, F("on"))) {
          success = P124_data->turn_on_channel(event->Par2);
        } else if (equals(subcommand, F("off"))) {
          success = P124_data->turn_off_channel(event->Par2);
        } else if (equals(subcommand, F("set"))) {
          success = P124_data->channelCtrl(event->Par2);
        } else if (equals(subcommand, F("get")) && (event->Par2 > 0) && (event->Par2 <= P124_CONFIG_RELAY_COUNT)) {
          uint8_t data = P124_data->getChannelState() & (1 << (event->Par2 - 1));
          UserVar[event->BaseVarIndex + 1] = event->Par2;
          UserVar[event->BaseVarIndex + 2] = (data ? 1 : 0);

          success = true;
        } else if (equals(subcommand, F("loop")) && (event->Par2 >= 0) && (event->Par2 <= 1)) {
          P124_data->setLoopState(event->Par2 == 1);

          success = true;
        }

        if (success) {
          UserVar[event->BaseVarIndex] = P124_data->getChannelState(); // Get relays state
        }
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P124
