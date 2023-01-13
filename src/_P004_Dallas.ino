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

// place sensor type selector right after the output value settings
# define P004_ERROR_STATE_OUTPUT PCONFIG(0)
# define P004_RESOLUTION         PCONFIG(1)
# define P004_SENSOR_TYPE_INDEX  2
# define P004_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P004_SENSOR_TYPE_INDEX)))

// Used to easily replace a sensor, without configuring.
// Can only be used for a single instance of this plugin and a single sensor.
# define P004_SCAN_ON_INIT       PCONFIG(3)


boolean Plugin_004(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_004;
      Device[deviceCount].Type               = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Simple;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_004);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      ExtraTaskSettings.populateDeviceValueNamesSeq(F("Temperature"), P004_NR_OUTPUT_VALUES, 2, false);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P004_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P004_SENSOR_TYPE_INDEX));
      event->idx        = P004_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(P004_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_RX(false);
      event->String2 = formatGpioName_TX(true);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      int8_t Plugin_004_DallasPin_RX = CONFIG_PIN1;
      int8_t Plugin_004_DallasPin_TX = CONFIG_PIN2;

      if (Plugin_004_DallasPin_TX == -1) {
        Plugin_004_DallasPin_TX = Plugin_004_DallasPin_RX;
      }

      if (validGpio(Plugin_004_DallasPin_RX) && validGpio(Plugin_004_DallasPin_TX)) {
        addFormCheckBox(F("Auto Select Sensor"), F("autoselect"), P004_SCAN_ON_INIT, P004_NR_OUTPUT_VALUES > 1);
        addFormNote(F("Auto Select can only be used for 1 Dallas sensor per GPIO pin."));
        Dallas_addr_selector_webform_load(event->TaskIndex, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX, P004_NR_OUTPUT_VALUES);

        {
          // Device Resolution select
          int activeRes =  P004_RESOLUTION;

          uint8_t savedAddress[8];
          Dallas_plugin_get_addr(savedAddress, event->TaskIndex);

          if (savedAddress[0] != 0) {
            activeRes = Dallas_getResolution(savedAddress, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX);
          }

          int resolutionChoice = P004_RESOLUTION;

          if ((resolutionChoice < 9) || (resolutionChoice > 12)) { resolutionChoice = activeRes; }
          const __FlashStringHelper *resultsOptions[4] = { F("9"), F("10"), F("11"), F("12") };
          int resultsOptionValues[4]                   = { 9, 10, 11, 12 };
          addFormSelector(F("Device Resolution"), F("res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
          addHtml(F(" Bit"));
        }

        {
          // Value in case of Error
          const __FlashStringHelper * resultsOptions[5]      = { F("NaN"), F("-127"), F("0"), F("125"), F("Ignore") };
          int    resultsOptionValues[5] = { P004_ERROR_NAN, P004_ERROR_MIN_RANGE, P004_ERROR_ZERO, P004_ERROR_MAX_RANGE, P004_ERROR_IGNORE };
          addFormSelector(F("Error State Value"), F("err"), 5, resultsOptions, resultsOptionValues, P004_ERROR_STATE_OUTPUT);
        }
        addFormNote(F("External pull up resistor is needed, see docs!"));

        {
          P004_data_struct *P004_data =
            static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P004_data) {
            for (uint8_t i = 0; i < P004_NR_OUTPUT_VALUES; ++i) {
              if (i == 0) {
                addFormSubHeader(F("Statistics"));
              } else {
                addFormSeparator(2);
              }
              Dallas_show_sensor_stats_webform_load(P004_data->get_sensor_data(i));
            }
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      int8_t Plugin_004_DallasPin_RX = CONFIG_PIN1;
      int8_t Plugin_004_DallasPin_TX = CONFIG_PIN2;

      if (Plugin_004_DallasPin_TX == -1) {
        Plugin_004_DallasPin_TX = Plugin_004_DallasPin_RX;
      }

      if (validGpio(Plugin_004_DallasPin_RX) && validGpio(Plugin_004_DallasPin_TX)) {
        // save the address for selected device and store into extra tasksettings
        Dallas_addr_selector_webform_save(event->TaskIndex, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX, P004_NR_OUTPUT_VALUES);

        uint8_t res = getFormItemInt(F("res"));

        if ((res < 9) || (res > 12)) { res = 12; }
        P004_RESOLUTION = res;

        uint8_t savedAddress[8];
        Dallas_plugin_get_addr(savedAddress, event->TaskIndex);
        Dallas_setResolution(savedAddress, res, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX);
      }
      P004_SCAN_ON_INIT       = isFormItemChecked(F("autoselect"));
      P004_ERROR_STATE_OUTPUT = getFormItemInt(F("err"));
      success                 = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P004_NR_OUTPUT_VALUES) {
          if (i != 0) {
            string += F("<br>");
          }

          if (nullptr != P004_data) {
            // Show the actively used IDs
            // For "Auto Select Sensor" no value is stored
            string += P004_data->get_formatted_address(i);
          } else {
            // Read the data from the settings.
            uint8_t addr[8]{};
            Dallas_plugin_get_addr(addr, event->TaskIndex, i);

            string += Dallas_format_address(addr);
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      int8_t Plugin_004_DallasPin_RX = CONFIG_PIN1;
      int8_t Plugin_004_DallasPin_TX = CONFIG_PIN2;
      const uint8_t res              = P004_RESOLUTION;

      if (Plugin_004_DallasPin_TX == -1) {
        Plugin_004_DallasPin_TX = Plugin_004_DallasPin_RX;
      }

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P004_data_struct(
        event->TaskIndex,
        Plugin_004_DallasPin_RX, 
        Plugin_004_DallasPin_TX, 
        res, 
        P004_NR_OUTPUT_VALUES == 1 && P004_SCAN_ON_INIT));
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P004_data) {
        for (uint8_t i = 0; i < P004_NR_OUTPUT_VALUES; ++i) {
          uint8_t addr[8] = { 0 };
          Dallas_plugin_get_addr(addr, event->TaskIndex, i);
          P004_data->add_addr(addr, i);
        }
        P004_data->init();
        success = true;
      }

      break;
    }

    case PLUGIN_READ:
    {
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P004_data) {
        if (P004_NR_OUTPUT_VALUES == 1 && P004_SCAN_ON_INIT) {
          if (!P004_data->sensorAddressSet()) {
            P004_data->init();
          }
        }
        if (!timeOutReached(P004_data->get_timer())) {
          Scheduler.schedule_task_device_timer(event->TaskIndex, P004_data->get_timer());
        } else {
          if (!P004_data->measurement_active()) {
            if (P004_data->initiate_read()) {
              Scheduler.schedule_task_device_timer(event->TaskIndex, P004_data->get_timer());
            }
          } else {
            // Try to get in sync with the existing interval again.
            Scheduler.reschedule_task_device_timer(event->TaskIndex, P004_data->get_measurement_start());

            P004_data->collect_values();

            for (uint8_t i = 0; i < P004_NR_OUTPUT_VALUES; ++i) {
              float value = 0;

              if (P004_data->read_temp(value, i))
              {
                UserVar[event->BaseVarIndex + i] = value;
                success                          = true;
              }
              else
              {
                if (P004_ERROR_STATE_OUTPUT != P004_ERROR_IGNORE) {
                  float errorValue = NAN;

                  switch (P004_ERROR_STATE_OUTPUT) {
                    case P004_ERROR_MIN_RANGE: errorValue = -127; break;
                    case P004_ERROR_ZERO:      errorValue = 0; break;
                    case P004_ERROR_MAX_RANGE: errorValue = 125; break;
                    default:
                      break;
                  }
                  UserVar[event->BaseVarIndex + i] = errorValue;
                }
              }

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("DS   : Temperature: ");

                if (success) {
                  log += formatUserVarNoCheck(event, i);
                } else {
                  log += F("Error!");
                }
                log += F(" (");
                log += P004_data->get_formatted_address(i);
                log += ')';
                addLogMove(LOG_LEVEL_INFO, log);
              }
            }
            P004_data->set_measurement_inactive();
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P004
