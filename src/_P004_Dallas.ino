#include "_Plugin_Helper.h"
#ifdef USES_P004

// #######################################################################################################
// #################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS18B20 datasheet : https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

/** Changelog:
 * 2024-05-11 tonhuisman: Add Get Config Value support for sensor statistics: Read success, Read retry, Read failed,
 *                        Read init failed, Resolution and Address (formatted)
 *                        [<taskname>#sensorstats,<sensorindex>,success|retry|failed|initfailed|resolution|address]
 * 2023-04-18 tonhuisman: Add warning on statistics section for Parasite Powered sensors, as these are unsupported.
 * 2023-04-17 tonhuisman: Use actual sensor resolution, even when using multiple sensors with different resolutions
 * 2023-04-16 tonhuisman: Rename from DS18b20 to 1-Wire Temperature, as it supports several 1-Wire temperature sensors
 *                        Add support for fixed-resolution sensors like MAX31826
 * 2023-04-16 tonhuisman: Start using changelog
 */
# include "src/PluginStructs/P004_data_struct.h"
# include "src/Helpers/Dallas1WireHelper.h"


# define PLUGIN_004
# define PLUGIN_ID_004         4
# define PLUGIN_NAME_004       "Environment - 1-Wire Temperature"
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
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_004;
      dev.Type           = DEVICE_TYPE_DUAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.PluginStats    = true;
      dev.setPin2Direction(gpio_direction::gpio_output);
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

      const int valueCount = P004_NR_OUTPUT_VALUES;

      if (validGpio(Plugin_004_DallasPin_RX) && validGpio(Plugin_004_DallasPin_TX)) {
        addFormCheckBox(F("Auto Select Sensor"), F("autoselect"), P004_SCAN_ON_INIT, valueCount > 1);
        addFormNote(F("Auto Select can only be used for 1 Dallas sensor per GPIO pin."));
        Dallas_addr_selector_webform_load(event->TaskIndex, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX, valueCount);

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
          const __FlashStringHelper *resultsOptions[] = { F("9"), F("10"), F("11"), F("12") };
          const int resultsOptionValues[]             = { 9, 10, 11, 12 };
          constexpr size_t optionCount                = NR_ELEMENTS(resultsOptionValues);
          addFormSelector(F("Device Resolution"), F("res"), optionCount, resultsOptions, resultsOptionValues, resolutionChoice);
          addHtml(F(" Bit"));
        }

        {
          // Value in case of Error
          const __FlashStringHelper *resultsOptions[] = { F("NaN"), F("-127"), F("0"), F("125"), F("Ignore") };
          int resultsOptionValues[]                   =
          { P004_ERROR_NAN, P004_ERROR_MIN_RANGE, P004_ERROR_ZERO, P004_ERROR_MAX_RANGE, P004_ERROR_IGNORE };
          constexpr size_t optionCount = NR_ELEMENTS(resultsOptionValues);
          addFormSelector(F("Error State Value"), F("err"), optionCount, resultsOptions, resultsOptionValues, P004_ERROR_STATE_OUTPUT);
        }
        addFormNote(F("External pull up resistor is needed, see docs!"));

        {
          P004_data_struct *P004_data =
            static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P004_data) {
            for (uint8_t i = 0; i < valueCount; ++i) {
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
      int8_t Plugin_004_DallasPin_RX = CONFIG_PIN1;
      int8_t Plugin_004_DallasPin_TX = CONFIG_PIN2;
      const int valueCount           = P004_NR_OUTPUT_VALUES;

      if (Plugin_004_DallasPin_TX == -1) {
        Plugin_004_DallasPin_TX = Plugin_004_DallasPin_RX;
      }


      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
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
            bool    hasFixedResolution = false; // FIXME tonhuisman: Not sure if I _want_ to read the resolution here...
            Dallas_plugin_get_addr(addr, event->TaskIndex, i);
            Dallas_getResolution(addr, Plugin_004_DallasPin_RX, Plugin_004_DallasPin_TX, hasFixedResolution);

            string += Dallas_format_address(addr, hasFixedResolution);
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
      const int     valueCount       = P004_NR_OUTPUT_VALUES;

      if (Plugin_004_DallasPin_TX == -1) {
        Plugin_004_DallasPin_TX = Plugin_004_DallasPin_RX;
      }

      {
        # ifdef USE_SECOND_HEAP
        HeapSelectIram ephemeral;
        # endif // ifdef USE_SECOND_HEAP

        initPluginTaskData(event->TaskIndex, new (std::nothrow) P004_data_struct(
                             event->TaskIndex,
                             Plugin_004_DallasPin_RX,
                             Plugin_004_DallasPin_TX,
                             res,
                             valueCount == 1 && P004_SCAN_ON_INIT));
      }
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P004_data) {
        for (uint8_t i = 0; i < valueCount; ++i) {
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
        const int valueCount = P004_NR_OUTPUT_VALUES;

        if ((valueCount == 1) && P004_SCAN_ON_INIT) {
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

            for (uint8_t i = 0; i < valueCount; ++i) {
              float value = 0.0f;

              if (P004_data->read_temp(value, i))
              {
                UserVar.setFloat(event->TaskIndex, i, value);
                success = true;
              }
              else
              {
                if (P004_ERROR_STATE_OUTPUT != P004_ERROR_IGNORE) {
                  float errorValue = NAN;

                  switch (P004_ERROR_STATE_OUTPUT) {
                    case P004_ERROR_MIN_RANGE: errorValue = -127.0f; break;
                    case P004_ERROR_ZERO:      errorValue = 0.0f; break;
                    case P004_ERROR_MAX_RANGE: errorValue = 125.0f; break;
                    default:
                      break;
                  }
                  UserVar.setFloat(event->TaskIndex, i, errorValue);
                }
              }

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("DS   : Temperature: ");

                if (success) {
                  log += formatUserVarNoCheck(event, i);
                } else {
                  log += F("Error!");
                }
                log += strformat(F(" (%s)"), P004_data->get_formatted_address(i).c_str());
                addLogMove(LOG_LEVEL_INFO, log);
              }
            }
            P004_data->set_measurement_inactive();
          }
        }
      }
      break;
    }

    # if P004_FEATURE_GET_CONFIG_VALUE
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P004_data_struct *P004_data =
        static_cast<P004_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P004_data) {
        const String cmd = parseString(string, 1, '.');

        if (equals(cmd, F("sensorstats"))) { // To distinguish from 'DeviceStats'
          const String par1 = parseString(string, 2, '.');
          int32_t nPar1;

          if (validIntFromString(par1, nPar1) && (nPar1 > 0) && (nPar1 <= P004_NR_OUTPUT_VALUES)) {
            nPar1--; // From DeviceNr to array index
            const String subcmd          = parseString(string, 3, '.');
            Dallas_SensorData sensorData = P004_data->get_sensor_data(nPar1);
            success = true;

            if (equals(subcmd, F("success"))) {
              string = sensorData.read_success;
            }  else
            if (equals(subcmd, F("retry"))) {
              string = sensorData.read_retry;
            } else
            if (equals(subcmd, F("failed"))) {
              string = sensorData.read_failed;
            } else
            if (equals(subcmd, F("initfailed"))) {
              string = sensorData.start_read_failed;
            } else
            if (equals(subcmd, F("resolution"))) {
              string = sensorData.actual_res;
            } else
            if (equals(subcmd, F("address"))) {
              string = sensorData.get_formatted_address();
            } else
            { // Unsupported stat
              success = false;
            }
          }
        }
      }
      break;
    }
    # endif // if P004_FEATURE_GET_CONFIG_VALUE
  }
  return success;
}

#endif // USES_P004
