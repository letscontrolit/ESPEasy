#include "_Plugin_Helper.h"
#ifdef USES_P100

// #######################################################################################################
// #################################### Plugin 100: Counter Dallas DS2423  ###############################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS2423 datasheet : https://datasheets.maximintegrated.com/en/ds/DS2423.pdf

# include "src/Helpers/Dallas1WireHelper.h"

# define PLUGIN_100
# define PLUGIN_ID_100         100
# define PLUGIN_NAME_100       "Pulse Counter - DS2423 [TESTING]"
# define PLUGIN_VALUENAME1_100 "CountDelta"

boolean Plugin_100(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_100;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_100);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_100));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("1-Wire"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("External pull up resistor is needed, see docs!"));

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      int8_t Plugin_100_DallasPin = CONFIG_PIN1;

      if (Plugin_100_DallasPin != -1) {
        // get currently saved address
        uint8_t savedAddress[8];
        Plugin_100_get_addr(savedAddress, event->TaskIndex);

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p100_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Dallas_reset(Plugin_100_DallasPin);
        Dallas_reset_search();

        while (Dallas_search(tmpAddress, Plugin_100_DallasPin))
        {
          String option = "";

          for (byte j = 0; j < 8; j++)
          {
            option += String(tmpAddress[j], HEX);

            if (j < 7) { option += '-'; }
          }
          bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;
          addSelector_Item(option, count, selected, false, "");

          count++;
        }
        addSelector_Foot();

        // Counter select
        String resultsOptions[2]      = { F("A"), F("B") };
        int    resultsOptionValues[2] = { 0, 1 };
        addFormSelector(F("Counter"), F("p100_counter"), 2, resultsOptions, resultsOptionValues, PCONFIG(0));
        addFormNote(F("Counter value is incremental"));
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // 1-wire GPIO
      int8_t Plugin_100_DallasPin = CONFIG_PIN1;

      // Counter choice
      PCONFIG(0) = getFormItemInt(F("p100_counter"));

      // 1-wire device address
      if (Plugin_100_DallasPin != -1) {
        uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        Dallas_scan(getFormItemInt(F("p100_dev")), addr, Plugin_100_DallasPin);

        for (byte x = 0; x < 8; x++) {
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      LoadTaskSettings(event->TaskIndex);
      uint8_t addr[8];
      Plugin_100_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      UserVar[event->BaseVarIndex]     = 0;
      UserVar[event->BaseVarIndex + 1] = 0;
      UserVar[event->BaseVarIndex + 2] = 0;

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      uint8_t addr[8];
      Plugin_100_get_addr(addr, event->TaskIndex);

      if (addr[0] != 0) {
        if (CONFIG_PIN1 != -1) {
          float value = 0;

          if (Dallas_readCounter(addr, &value, CONFIG_PIN1, PCONFIG(0)))
          {
            UserVar[event->BaseVarIndex] = UserVar[event->BaseVarIndex + 2] != 0
              ? value - UserVar[event->BaseVarIndex + 1]
              : 0;
            UserVar[event->BaseVarIndex + 2] = 1;
            UserVar[event->BaseVarIndex + 1] = value;
            success                          = true;
          }
          else
          {
            UserVar[event->BaseVarIndex] = NAN;
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("[P100]DS   : Counter ");
            log += PCONFIG(0) == 0 ? F("A") : F("B");
            log += F(": ");

            if (success) {
              log += UserVar[event->BaseVarIndex];
            } else {
              log += F("Error!");
            }
            log += F(" (");
            log += Dallas_format_address(addr);
            log += ')';
            addLog(LOG_LEVEL_INFO, log);
          }
        }
      }
      break;
    }
  }
  return success;
}

// Load ROM address from tasksettings
void Plugin_100_get_addr(uint8_t addr[], taskIndex_t TaskIndex)
{
  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (byte x = 0; x < 8; x++) {
    addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
  }
}

#endif // USES_P100
