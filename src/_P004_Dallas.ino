#include "_Plugin_Helper.h"
#ifdef USES_P004

// #######################################################################################################
// #################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS18B20 datasheet : https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

# include "src/PluginStructs/P004_data_struct.h"
# include "src/Helpers/Dallas1WireHelper.h"


# define PLUGIN_004
# define PLUGIN_ID_004         4
# define PLUGIN_NAME_004       "Environment - DS18b20"
# define PLUGIN_VALUENAME1_004 "Temperature"

# define P004_ERROR_NAN        0
# define P004_ERROR_MIN_RANGE  1
# define P004_ERROR_ZERO       2
# define P004_ERROR_MAX_RANGE  3
# define P004_ERROR_IGNORE     4


boolean Plugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_004;
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
      string = F(PLUGIN_NAME_004);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_004));
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
      int8_t Plugin_004_DallasPin = CONFIG_PIN1;

      if (Plugin_004_DallasPin != -1) {
        // get currently saved address
        uint8_t savedAddress[8];
        Plugin_004_get_addr(savedAddress, event->TaskIndex);

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p004_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Dallas_reset(Plugin_004_DallasPin);
        Dallas_reset_search();

        while (Dallas_search(tmpAddress, Plugin_004_DallasPin))
        {
          String option = Dallas_format_address(tmpAddress);

          bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;
          addSelector_Item(option, count, selected, false, "");
          count++;
        }
        addSelector_Foot();

        {
          // Device Resolution select
          int activeRes =  PCONFIG(1);

          if (savedAddress[0] != 0) {
            activeRes = Dallas_getResolution(savedAddress, Plugin_004_DallasPin);
          }

          int resolutionChoice = PCONFIG(1);

          if ((resolutionChoice < 9) || (resolutionChoice > 12)) { resolutionChoice = activeRes; }
          String resultsOptions[4]      = { F("9"), F("10"), F("11"), F("12") };
          int    resultsOptionValues[4] = { 9, 10, 11, 12 };
          addFormSelector(F("Device Resolution"), F("p004_res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
          addHtml(F(" Bit"));

          if (savedAddress[0] != 0) {
            String note = F("Active resolution: ");
            note += activeRes;
            note += F(" bit");
            addFormNote(note);
          }
        }

        {
          // Value in case of Error
          String resultsOptions[5]      = { F("NaN"), F("-127"), F("0"), F("125"), F("Ignore") };
          int    resultsOptionValues[5] = { P004_ERROR_NAN, P004_ERROR_MIN_RANGE, P004_ERROR_ZERO, P004_ERROR_MAX_RANGE, P004_ERROR_IGNORE };
          addFormSelector(F("Error State Value"), F("p004_err"), 5, resultsOptions, resultsOptionValues, PCONFIG(0));
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      int8_t Plugin_004_DallasPin = CONFIG_PIN1;

      if (Plugin_004_DallasPin != -1) {
        // save the address for selected device and store into extra tasksettings
        LoadTaskSettings(event->TaskIndex);
        uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        Dallas_scan(getFormItemInt(F("p004_dev")), addr, Plugin_004_DallasPin);

        for (byte x = 0; x < 8; x++) {
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        }

        uint8_t res = getFormItemInt(F("p004_res"));

        if ((res < 9) || (res > 12)) { res = 12; }
        PCONFIG(1) = res;

        Dallas_setResolution(addr, res, Plugin_004_DallasPin);
      }
      PCONFIG(0) = getFormItemInt(F("p004_err"));
      success    = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      LoadTaskSettings(event->TaskIndex);
      uint8_t addr[8];
      Plugin_004_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      uint8_t addr[8];
      Plugin_004_get_addr(addr, event->TaskIndex);

      if ((addr[0] != 0) && (CONFIG_PIN1 != -1)) {
        const uint8_t res = PCONFIG(1);
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P004_data_struct(CONFIG_PIN1, addr, res));
        P004_data_struct *P004_data =
          static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P004_data) {
          success = true;
        }
      }

      break;
    }

    case PLUGIN_READ:
    {
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P004_data) {
        if (!timeOutReached(P004_data->get_timer())) {
          Scheduler.schedule_task_device_timer(event->TaskIndex, P004_data->get_timer());
        } else {
          if (!P004_data->measurement_active()) {
            if (P004_data->initiate_read()) {
              Scheduler.schedule_task_device_timer(event->TaskIndex, P004_data->get_timer());
            }
          } else {
            P004_data->set_measurement_inactive();

            // Try to get in sync with the existing interval again.
            Scheduler.reschedule_task_device_timer(event->TaskIndex, P004_data->get_measurement_start());
            float value = 0;

            if (P004_data->read_temp(value))
            {
              UserVar[event->BaseVarIndex] = value;
              success                      = true;
            }
            else
            {
              if (PCONFIG(0) != P004_ERROR_IGNORE) {
                float errorValue = NAN;

                switch (PCONFIG(0)) {
                  case P004_ERROR_MIN_RANGE: errorValue = -127; break;
                  case P004_ERROR_ZERO:      errorValue = 0; break;
                  case P004_ERROR_MAX_RANGE: errorValue = 125; break;
                  default:
                    break;
                }
                UserVar[event->BaseVarIndex] = errorValue;
              }
            }

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("DS   : Temperature: ");

              if (success) {
                log += UserVar[event->BaseVarIndex];
              } else {
                log += F("Error!");
              }
              log += F(" (");
              log += P004_data->get_formatted_address();
              log += ')';
              addLog(LOG_LEVEL_INFO, log);
            }
          }
        }
      }
      break;
    }
  }
  return success;
}

// Load ROM address from tasksettings
void Plugin_004_get_addr(uint8_t addr[], taskIndex_t TaskIndex)
{
  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (byte x = 0; x < 8; x++) {
    addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
  }
}

#endif // USES_P004
