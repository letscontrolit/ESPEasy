#include "_Plugin_Helper.h"

#ifdef USES_P124

// #######################################################################################################
// ########################### Plugin 124 I2C Multi Relay module      ###############################
// #######################################################################################################

/** Changelog:
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
# define PLUGIN_NAME_124        "Output - I2C Multi Relay [TESTING]"
# define PLUGIN_VALUENAME1_124  "State"

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
      Device[deviceCount].ValueCount         = 1;
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
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x11);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // No decimals needed
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *optionsMode2[] = {
        F("2 relays"),
        F("4 relays"),
        F("8 relays") };
      int optionValuesMode2[] { 2, 4, 8 };
      addFormSelector(F("Number of relays"), F("plugin_124_relays"), 3, optionsMode2, optionValuesMode2, P124_CONFIG_RELAY_COUNT, true);

      const __FlashStringHelper *yesNoOptions[] = {
        F("No"),
        F("Yes")
      };
      int yesNoValues[] = { 0, 1 };
      addFormSelector(F("Initialize relays on startup"),
                      getPluginCustomArgName(P124_FLAGS_INIT_RELAYS), 2, yesNoOptions, yesNoValues,
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
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P124_CONFIG_RELAY_COUNT = getFormItemInt(F("plugin_124_relays"));
      uint32_t lSettings = 0u;
      bitWrite(lSettings, P124_FLAGS_INIT_RELAYS, getFormItemInt(getPluginCustomArgName(P124_FLAGS_INIT_RELAYS)) == 1);
      bitWrite(lSettings, P124_FLAGS_INIT_ALWAYS, getFormItemInt(getPluginCustomArgName(P124_FLAGS_INIT_ALWAYS)) == 1);

      if (lSettings != 0) {
        for (int i = 0; i < P124_CONFIG_RELAY_COUNT; i++) {
          bitWrite(lSettings, i, isFormItemChecked(getPluginCustomArgName(i)));
        }
      }
      P124_CONFIG_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P124_data_struct(PCONFIG(0)));
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P124_data) {
        return success;
      }

      if (P124_data->isInitialized()) {
        String log;
        log.reserve(46);
        log  = F("MultiRelay: Initialized, firmware version: ");
        log += P124_data->getFirmwareVersion();
        addLog(LOG_LEVEL_INFO, log);

        if (bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_RELAYS) &&
            (!bitRead(P124_InitializedRelays, event->TaskIndex) ||
             bitRead(P124_CONFIG_FLAGS, P124_FLAGS_INIT_ALWAYS))) {
          P124_data->channelCtrl(P124_CONFIG_FLAGS & 0xFF);            // Set relays state
          UserVar[event->BaseVarIndex] = P124_data->getChannelState(); // Get relays state
          bitSet(P124_InitializedRelays, event->TaskIndex);            // Update initialization status
        }
        success = true;
      } else {
        addLog(LOG_LEVEL_ERROR, F("MultiRelay: Initialization error!"));
      }

      break;
    }

    case PLUGIN_READ:
    {
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P124_data) {
        return success;
      }

      if (P124_data->isInitialized()) {
        UserVar[event->BaseVarIndex] = P124_data->getChannelState(); // Get relays state
        success                      = true;
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P124_data) && P124_data->isInitialized()) {
        uint8_t varNr = 1; // VARS_PER_TASK;
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
      P124_data_struct *P124_data = static_cast<P124_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P124_data) {
        return success;
      }

      # ifdef P124_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, string);
      String log = F("Par1..3:");
      log += event->Par1;
      log += ',';
      log += event->Par2;
      log += ',';
      log += event->Par3;
      addLog(LOG_LEVEL_INFO, log);
      # endif // ifdef P124_DEBUG_LOG

      String command = parseString(string, 1);

      if (P124_data->isInitialized() &&
          command.equals(F("multirelay"))) {
        String subcommand = parseString(string, 2);

        if (subcommand.equals(F("on"))) {
          success = P124_data->turn_on_channel(event->Par2);
        } else if (subcommand.equals(F("off"))) {
          success = P124_data->turn_off_channel(event->Par2);
        } else if (subcommand.equals(F("set"))) {
          success = P124_data->channelCtrl(event->Par2);
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
