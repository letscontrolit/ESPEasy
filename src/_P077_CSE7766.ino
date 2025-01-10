#include "_Plugin_Helper.h"
#ifdef USES_P077

// #######################################################################################################
// ###################### Plugin 077: CSE7766 - Energy (Sonoff S31 and Sonoff Pow R2) ####################
// #######################################################################################################
// ###################################### stefan@clumsy.ch      ##########################################
// #######################################################################################################

/** Changelog:
 * 2023-02-12 tonhuisman: Separate PLUGIN_SERIAL_IN and PLUGIN_TEN_PER_SECOND (changed from PLUGIN_FIFTY_PER_SECOND) handling
 *                        Fixed some minor code-issues
 * 2023-02-11 tonhuisman: Add PLUGIN_WRITE support for csereset and csecalibrate,[Voltage],[Current],[Power]
 *                        Handle serial input also in PLUGIN_FIFTY_PER_SECOND, so other configurations than
 *                        HWSerial0 will also be processed
 *                        Add labels for RX/TX GPIO pins
 * 2023-02-10 tonhuisman: Move from HWSerial0 to EasySerial to allow flexible serial configuration
 * 2023-02-10 tonhuisman: Add changelog
 */

# include "src/PluginStructs/P077_data_struct.h"

# define PLUGIN_077
# define PLUGIN_ID_077            77
# ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766 (POW r2)"
# else // ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766"
# endif // ifdef PLUGIN_SET_SONOFF_POW


# define P077_QUERY1_DFLT         P077_query::P077_QUERY_VOLTAGE
# define P077_QUERY2_DFLT         P077_query::P077_QUERY_ACTIVE_POWER
# define P077_QUERY3_DFLT         P077_query::P077_QUERY_CURRENT
# define P077_QUERY4_DFLT         P077_query::P077_QUERY_PULSES


boolean Plugin_077(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_077;
      dev.Type             = DEVICE_TYPE_SERIAL;
      dev.VType            = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption    = true;
      dev.ValueCount       = 4;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.TimerOptional    = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_077);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < static_cast<uint8_t>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS)) {
          const uint8_t pconfigIndex = i + P077_QUERY1_CONFIG_POS;
          P077_query    choice       = static_cast<P077_query>(PCONFIG(pconfigIndex));
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_077_valuename(choice, false));

          if (choice == P077_query::P077_QUERY_PULSES) {
            ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
          }
          else if (choice == P077_query::P077_QUERY_KWH) {
            ExtraTaskSettings.TaskDeviceValueDecimals[i] = 3;
          }
          else {
            ExtraTaskSettings.TaskDeviceValueDecimals[i] = 2;
          }
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      CONFIG_PIN1 = 3; // Former default HWSerial0
      CONFIG_PIN2 = 1;
      CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial0);

      P077_QUERY1 = static_cast<uint8_t>(P077_QUERY1_DFLT);
      P077_QUERY2 = static_cast<uint8_t>(P077_QUERY2_DFLT);
      P077_QUERY3 = static_cast<uint8_t>(P077_QUERY3_DFLT);
      P077_QUERY4 = static_cast<uint8_t>(P077_QUERY4_DFLT);

      success = true;
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P077_data_struct *P077_data =
        static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P077_data) && P077_data->isInitialized()) {
        const P077_query query = Plugin_077_from_valuename(string);

        if (query != P077_query::P077_QUERY_NR_OUTPUT_OPTIONS) {
          const float value = P077_data->getValue(query);
          int nrDecimals    = 2;

          if (query == P077_query::P077_QUERY_PULSES) {
            nrDecimals = 0;
          } else if (query == P077_query::P077_QUERY_KWH) {
            nrDecimals = 3;
          }

          string  = toString(value, nrDecimals);
          success = true;
        }
      }
      break;
    }


    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      if ((CONFIG_PIN1 == -1) && (CONFIG_PIN2 == -1) && (CONFIG_PORT == 0)) {
        CONFIG_PIN1 = 3; // Former default HWSerial0
        CONFIG_PIN2 = 1;
        CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial0);
      }
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *options[static_cast<uint8_t>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS)];

      for (uint8_t i = 0; i < static_cast<uint8_t>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS); ++i) {
        options[i] = Plugin_077_valuename(static_cast<P077_query>(i), true);
      }

      for (uint8_t i = 0; i < P077_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P077_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, static_cast<int>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS), options);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormNumericBox(F("U Ref"), P077_UREF_LABEL, P077_UREF);
      addUnit(F("uSec"));

      addFormNumericBox(F("I Ref"), P077_IREF_LABEL, P077_IREF);
      addUnit(F("uSec"));

      addFormNumericBox(F("P Ref"), P077_PREF_LABEL, P077_PREF);
      addUnit(F("uSec"));
      addFormNote(F("Use 0 to read values stored on chip / default values"));

      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        addRowLabel(F("Pulses per kWh"));
        const int pulsesPerKwh = P077_data->cf_frequency * 3600;
        addHtmlInt(pulsesPerKwh);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P077_UREF = getFormItemInt(P077_UREF_LABEL);
      P077_IREF = getFormItemInt(P077_IREF_LABEL);
      P077_PREF = getFormItemInt(P077_PREF_LABEL);

      // Save output selector parameters.
      for (int i = 0; i < P077_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P077_QUERY1_CONFIG_POS;
        const P077_query choice    = static_cast<P077_query>(PCONFIG(pconfigIndex));
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_077_valuename(choice, false));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      if ((CONFIG_PIN1 == -1) && (CONFIG_PIN2 == -1) && (CONFIG_PORT == 0)) {
        CONFIG_PIN1 = 3; // Former default HWSerial0
        CONFIG_PIN2 = 1;
        CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial0);
      }
      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      if ((P077_QUERY1 == 0) &&
          (P077_QUERY2 == 0) &&
          (P077_QUERY3 == 0) &&
          (P077_QUERY4 == 0))
      {
        // Set default output selection
        P077_QUERY1 = static_cast<uint8_t>(P077_QUERY1_DFLT);
        P077_QUERY2 = static_cast<uint8_t>(P077_QUERY2_DFLT);
        P077_QUERY3 = static_cast<uint8_t>(P077_QUERY3_DFLT);
        P077_QUERY4 = static_cast<uint8_t>(P077_QUERY4_DFLT);
      }

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P077_data_struct());
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P077_data) {
        return success;
      }

      if (P077_UREF == 0) { P077_UREF = CSE_UREF_PULSE; }

      if (P077_IREF == 0) { P077_IREF = CSE_IREF_PULSE; }

      if (P077_PREF == 0) { P077_PREF = CSE_PREF_PULSE; }

      if (P077_data->init(port, serial_rx, serial_tx, 4800, static_cast<uint8_t>(SERIAL_8E1))) {
        success = true;
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
      }

      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        uint8_t varNr            = VARS_PER_TASK;
        const float pulsesPerKwh = P077_data->cf_frequency * 3600;
        const float kWh          = P077_data->cf_pulses / pulsesPerKwh;
        pluginWebformShowValue(event->TaskIndex, varNr++, F("kWh"), toString(kWh, 3), true);
      }
      break;
    }

    case PLUGIN_READ: {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG_DEV, F("CSE: plugin read"));
      # endif // ifndef BUILD_NO_DEBUG

      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        // Variables set in PLUGIN_SERIAL_IN/PLUGIN_TEN_PER_SECOND as soon as there are new values!
        // Update when there is a new value
        success = P077_data->plugin_read(event);
      }
      break;
    }

    case PLUGIN_SERIAL_IN: // When using HWSerial0

      if (ESPEasySerialPort::serial0 != static_cast<ESPEasySerialPort>(CONFIG_PORT)) {
        return success;
      }

    // fallthrough
    case PLUGIN_TEN_PER_SECOND: // When using other than HWSerial0
    {
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        success = true;

        /* ONLINE CHECKSUMMING by Bartłomiej Zimoń */
        if (P077_data->processSerialData()) {
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("CSE: packet found"));
          # endif // ifndef BUILD_NO_DEBUG

          if (P077_data->processCseReceived(event)) {
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CSE voltage: "), P077_data->getValue(P077_query::P077_QUERY_VOLTAGE)));
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CSE power: "), P077_data->getValue(P077_query::P077_QUERY_ACTIVE_POWER)));
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CSE current: "), P077_data->getValue(P077_query::P077_QUERY_CURRENT)));
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CSE pulses: "), P077_data->cf_pulses));
            }
            # endif // ifndef BUILD_NO_DEBUG
          }

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLogMove(LOG_LEVEL_DEBUG,
                       strformat(F("CSE: time %d/%d/%d"),
                                 P077_data->t_max, P077_data->t_pkt, P077_data->t_all));
            addLogMove(LOG_LEVEL_DEBUG,
                       strformat(F("CSE: bytes %d/%d/%d"),
                                 P077_data->count_bytes, P077_data->count_max, P077_data->serial_Available()));
            addLogMove(LOG_LEVEL_DEBUG,
                       concat(F("CSE: nr "), P077_data->count_pkt));
          }
          # endif // ifndef BUILD_NO_DEBUG
          P077_data->t_all       = 0;
          P077_data->count_bytes = 0;
        }
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        success = P077_data->plugin_write(event, string);
      }
      break;
    }
  }
  return success;
}

#endif // USES_P077
