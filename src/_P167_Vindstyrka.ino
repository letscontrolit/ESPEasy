#include "_Plugin_Helper.h"
#ifdef USES_P167

// #######################################################################################################
// ########################   Plugin 167 IKEA Vindstyrka I2C  Sensor (SEN5x)  ############################
// #######################################################################################################

/** Changelog:
 * 2024-05-05 tonhuisman: Add subcommand sen5x,techlog,<1|0> to enable/disable Technical logging option. 0 = Off, any other value is on
 * 2024-04-20 tonhuisman: Replace dewpoint calculation by standard calculation, fix issue with status bits, reduce strings
 *                        Remove unneeded code and variables, move most defines to P167_data_struct.h
 *                        Implement Get Config Value to retrieve all available values from a single instance
 *                        Implement multi-instance use (using an I2C multiplexer, as the address isn't configurable)
 *                        Use enum classes where applicable
 *                        Keeping the FSM in place
 * 2024-04-19 tonhuisman: Source formatting using Uncrustify (ESPEasy standard) and string handling modifications
 * 2023-06-19 AndiBaciu creation based upon https://github.com/RobTillaart/SHT2x
 */

# include "./src/PluginStructs/P167_data_struct.h"

# define PLUGIN_167
# define PLUGIN_ID_167     167                                               // plugin id
# define PLUGIN_NAME_167   "Environment - Sensirion SEN5x (IKEA Vindstyrka)" // What will be dislpayed in the selection list


boolean Plugin_167(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_167;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption    = true;
      dev.ValueCount       = 4;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.I2CNoDeviceCheck = true;
      dev.I2CMax100kHz     = true; // SEN5x only supports up to 100 kHz
      dev.PluginStats      = true;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      break;
    }


    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_167);
      break;
    }


    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      if (P167_I2C_ADDRESS_DFLT == PCONFIG(0)) {
        PCONFIG(P167_SENSOR_TYPE_INDEX) = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_QUAD);
      }
      event->Par1 = P167_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }


    case PLUGIN_GET_DEVICEVTYPE:
    {
      if (P167_I2C_ADDRESS_DFLT == PCONFIG(0)) {
        PCONFIG(P167_SENSOR_TYPE_INDEX) = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_QUAD);
      }
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P167_SENSOR_TYPE_INDEX));
      event->idx        = P167_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }


    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      if (P167_I2C_ADDRESS_DFLT == PCONFIG(0)) {
        PCONFIG(P167_SENSOR_TYPE_INDEX) = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_QUAD);
      }

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P167_NR_OUTPUT_VALUES) {
          uint8_t choice = PCONFIG(i + P167_QUERY1_CONFIG_POS);
          safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[i], P167_getQueryValueString(choice),
                       sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      P167_MODEL                      = P167_MODEL_DFLT;
      P167_QUERY1                     = P167_QUERY1_DFLT;
      P167_QUERY2                     = P167_QUERY2_DFLT;
      P167_QUERY3                     = P167_QUERY3_DFLT;
      P167_QUERY4                     = P167_QUERY4_DFLT;
      P167_MON_SCL_PIN                = P167_MON_SCL_PIN_DFLT;
      PCONFIG(P167_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);

      success = true;
      break;
    }


    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P167_I2C_ADDRESS_DFLT;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS


    case PLUGIN_I2C_HAS_ADDRESS:

    {
      success = P167_I2C_ADDRESS_DFLT == event->Par1;
      break;
    }


    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      // if (P167_SEN_FIRST == event->TaskIndex) { // If first SEN, serial config available
      if (P167_MODEL == 0) {
        string = strformat(F("MonPin SCL: %s"), formatGpioLabel(P167_MON_SCL_PIN, false).c_str());
      }

      // }
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      if (P167_I2C_ADDRESS_DFLT == PCONFIG(0)) {
        PCONFIG(P167_SENSOR_TYPE_INDEX) = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_QUAD);
      }
      const __FlashStringHelper *options[P167_NR_OUTPUT_OPTIONS];

      for (int i = 0; i < P167_NR_OUTPUT_OPTIONS; ++i) {
        options[i] = P167_getQueryString(i);
      }

      for (uint8_t i = 0; i < P167_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P167_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P167_NR_OUTPUT_OPTIONS, options);
      }
      addFormNote(F("NOx is available ONLY on Sensirion SEN55 model"));
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options_model[] = {
        toString(P167_model::Vindstyrka),
        toString(P167_model::SEN54),
        toString(P167_model::SEN55),
      };
      const int options_model_value[] = {
        P167_MODEL_VINDSTYRKA,
        P167_MODEL_SEN54,
        P167_MODEL_SEN55,
      };
      constexpr uint8_t optCount = NR_ELEMENTS(options_model_value);

      addFormSelector(F("Model Type"), P167_MODEL_LABEL, optCount,
                      options_model, options_model_value, P167_MODEL, true);
      addFormNote(F("Changing the Model Type will reload the page."));

      if (P167_MODEL == P167_MODEL_VINDSTYRKA) {
        addFormPinSelect(PinSelectPurpose::Generic_input, F("MonPin SCL"), F("taskdevicepin3"), P167_MON_SCL_PIN);
        addFormNote(F("Pin for monitoring I2C communication between Vindstyrka controller and SEN54. "
                      "(Only when Model Type: IKEA Vindstyrka is selected.)"));
      }

      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (Plugin_167_SEN != nullptr) {
        addRowLabel(F("Device info"));
        String  prodname;
        String  sernum;
        uint8_t firmware;
        Plugin_167_SEN->getEID(prodname, sernum, firmware);
        addHtml(strformat(F("ProdName: %s, Serial Number: %s, Firmware: %d"),
                          prodname.c_str(), sernum.c_str(), firmware));

        addRowLabel(F("Device status"));
        addHtml(strformat(F("Speed warning: %d, Auto Cleaning: %d, GAS Error: %d, "
                            "RHT Error: %d, LASER Error: %d, FAN Error: %d"),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_speed),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_autoclean),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_gas),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_rht),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_laser),
                          Plugin_167_SEN->getStatusInfo(P167_statusinfo::sensor_fan)
                          ));

        addRowLabel(F("Check (pass/fail/errCode)"));
        addHtml(strformat(F("%d/%d/%d"),
                          Plugin_167_SEN->getSuccCount(),
                          Plugin_167_SEN->getErrCount(),
                          Plugin_167_SEN->getErrCode()
                          ));
      }

      addFormCheckBox(F("Technical logging"), P167_ENABLE_LOG_LABEL, P167_ENABLE_LOG);
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SAVE:
    {
      // Save output selector parameters.
      for (uint8_t i = 0; i < P167_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P167_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, P167_getQueryValueString(choice));
      }
      P167_MODEL      = getFormItemInt(P167_MODEL_LABEL);
      P167_ENABLE_LOG = isFormItemChecked(P167_ENABLE_LOG_LABEL);

      if (P167_MODEL == P167_MODEL_VINDSTYRKA) {
        P167_MON_SCL_PIN = getFormItemInt(F("taskdevicepin3"));
      } else {
        P167_MON_SCL_PIN = -1; // None
      }


      success = true;
      break;
    }


    case PLUGIN_INIT:
    {
      if (P167_I2C_ADDRESS_DFLT == PCONFIG(0)) {
        PCONFIG(P167_SENSOR_TYPE_INDEX) = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_QUAD);
      }
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P167_data_struct());
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (Plugin_167_SEN != nullptr) {
        Plugin_167_SEN->setupModel(static_cast<P167_model>(P167_MODEL));
        Plugin_167_SEN->setupDevice(P167_I2C_ADDRESS_DFLT);
        Plugin_167_SEN->setLogging(P167_ENABLE_LOG);

        if (P167_MODEL == P167_MODEL_VINDSTYRKA) {
          Plugin_167_SEN->setupMonPin(P167_MON_SCL_PIN);
        }
        success = Plugin_167_SEN->reset();
      }

      for (taskVarIndex_t v = 0; v < P167_NR_OUTPUT_VALUES; ++v) {
        UserVar.setFloat(event->TaskIndex, v, NAN);
      }

      break;
    }


    case PLUGIN_EXIT:
    {
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((Plugin_167_SEN != nullptr) && (P167_MODEL == P167_MODEL_VINDSTYRKA)) {
        Plugin_167_SEN->disableInterrupt_monpin();
      }

      success = true;
      break;
    }


    case PLUGIN_READ:
    {
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != Plugin_167_SEN) {
        if (Plugin_167_SEN->inError()) {
          for (taskVarIndex_t v = 0; v < P167_NR_OUTPUT_VALUES; ++v) {
            UserVar.setFloat(event->TaskIndex, v, NAN);
          }
          addLog(LOG_LEVEL_ERROR, F("Vindstyrka / SEN5X: in Error!"));
        } else {
          // if (event->TaskIndex == P167_SEN_FIRST) {
          Plugin_167_SEN->startMeasurements(); // getting ready for another read cycle
          // }

          for (taskVarIndex_t v = 0; v < P167_NR_OUTPUT_VALUES; ++v) {
            UserVar.setFloat(event->TaskIndex, v, Plugin_167_SEN->getRequestedValue(PCONFIG(P167_QUERY1_CONFIG_POS + v)));
          }
          success = true;
        }
      }

      break;
    }


    case PLUGIN_FIFTY_PER_SECOND:
    {
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != Plugin_167_SEN) {
        Plugin_167_SEN->monitorSCL(); // Vindstryka / SEN5X FSM evaluation
        Plugin_167_SEN->update();
      }

      // }
      success = true;
    }

    case PLUGIN_GET_CONFIG_VALUE:

    {
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != Plugin_167_SEN) {
        for (uint8_t v = 0; v < P167_VALUE_COUNT && !success; ++v) {
          if (string.equalsIgnoreCase(P167_getQueryValueString(v))) {
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLog(LOG_LEVEL_DEBUG, strformat(F("SEN5x: Get Config Value: %s: %.2f"),
                                                string.c_str(), Plugin_167_SEN->getRequestedValue(v)));
            }
            # endif // ifndef BUILD_NO_DEBUG
            string  = toString(Plugin_167_SEN->getRequestedValue(v));
            success = true;
            break;
          }
        }
      }
      break;
    }


    case PLUGIN_WRITE:
    {
      P167_data_struct *Plugin_167_SEN = static_cast<P167_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != Plugin_167_SEN) {
        const String cmd = parseString(string, 1);

        if (equals(cmd, F("sen5x"))) {
          const String subcmd = parseString(string, 2);

          if (equals(subcmd, F("startclean"))) {
            Plugin_167_SEN->startCleaning();
            success = true;
          } else
          if (equals(subcmd, F("techlog"))) {
            P167_ENABLE_LOG = event->Par2 == 0 ? 0 : 1;
            Plugin_167_SEN->setLogging(P167_ENABLE_LOG);
            success = true;
          }
        }
      }
      break;
    }
  } // switch

  return success;
}   // Plugin_167

#endif  // USES_P167
