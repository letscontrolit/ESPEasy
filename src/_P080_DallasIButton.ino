#include "_Plugin_Helper.h"
#ifdef USES_P080

// #######################################################################################################
// #################################### Plugin 080: iButton Sensor  DS1990A    ###########################
// #######################################################################################################

// Maxim Integrated

# include "src/Helpers/Dallas1WireHelper.h"

# define PLUGIN_080
# define PLUGIN_ID_080         80
# define PLUGIN_NAME_080       "Input - iButton [TESTING]"
# define PLUGIN_VALUENAME1_080 "iButton"


int8_t Plugin_080_DallasPin;

boolean Plugin_080(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_080;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_LONG;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_080);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_080));
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
      Plugin_080_DallasPin = CONFIG_PIN1;

      if (Plugin_080_DallasPin != -1) {
        // get currently saved address
        uint8_t savedAddress[8];
        Plugin_080_get_addr(savedAddress, event->TaskIndex);

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p080_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Dallas_reset(Plugin_080_DallasPin);
        Dallas_reset_search();

        while (Dallas_search(tmpAddress, Plugin_080_DallasPin))
        {
          String option   = Dallas_format_address(tmpAddress);
          bool   selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;

          // check for DS1990A
          if (tmpAddress[0] == 0x01) {
            addSelector_Item(option, count, selected, false, "");
          }


          count++;
        }
        addSelector_Foot();
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // save the address for selected device and store into extra tasksettings
      Plugin_080_DallasPin = CONFIG_PIN1;

      // byte devCount =
      if (Plugin_080_DallasPin != -1) {
        uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        Dallas_scan(getFormItemInt(F("p080_dev")), addr, Plugin_080_DallasPin);

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
      Plugin_080_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      Plugin_080_DallasPin = CONFIG_PIN1;

      if (Plugin_080_DallasPin != -1) {
        uint8_t addr[8];
        Plugin_080_get_addr(addr, event->TaskIndex);
        Dallas_startConversion(addr, Plugin_080_DallasPin);

        delay(800); // give it time to do intial conversion
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // PLUGIN_READ:
    {
      uint8_t addr[8];
      Plugin_080_get_addr(addr, event->TaskIndex);

      if (addr[0] != 0) {
        Plugin_080_DallasPin = CONFIG_PIN1;

        if (Dallas_readiButton(addr, Plugin_080_DallasPin))
        {
          UserVar[event->BaseVarIndex] = 1;
          success                      = true;
        }
        else
        {
          UserVar[event->BaseVarIndex] = 0;
        }
        Dallas_startConversion(addr, Plugin_080_DallasPin);

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("DS   : iButton: ");

          if (success) {
            log += UserVar[event->BaseVarIndex];
          } else {
            log += F("Not Present!");
          }
          addLog(LOG_LEVEL_DEBUG, log);
        }
      }
      break;
    }
  }
  return success;
}

void Plugin_080_get_addr(uint8_t addr[], taskIndex_t TaskIndex)
{
  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (byte x = 0; x < 8; x++) {
    addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
  }
}

#endif // USES_P080
