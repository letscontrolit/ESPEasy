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
# define PLUGIN_ID_077         77
# ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766 (POW r2)"
# else // ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766"
# endif // ifdef PLUGIN_SET_SONOFF_POW
# define PLUGIN_VALUENAME1_077 "Voltage"
# define PLUGIN_VALUENAME2_077 "Power"
# define PLUGIN_VALUENAME3_077 "Current"
# define PLUGIN_VALUENAME4_077 "Pulses"


boolean Plugin_077(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_077;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_077);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_077));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_077));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_077));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_077));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      CONFIG_PIN1 = 3; // Former default HWSerial0
      CONFIG_PIN2 = 1;
      CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial0);
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

    case PLUGIN_WEBFORM_LOAD: {
      addFormNumericBox(F("U Ref"), F("URef"), PCONFIG(0));
      addUnit(F("uSec"));

      addFormNumericBox(F("I Ref"), F("IRef"), PCONFIG(1));
      addUnit(F("uSec"));

      addFormNumericBox(F("P Ref"), F("PRef"), PCONFIG(2));
      addUnit(F("uSec"));
      addFormNote(F("Use 0 to read values stored on chip / default values"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      PCONFIG(0) = getFormItemInt(F("URef"));
      PCONFIG(1) = getFormItemInt(F("IRef"));
      PCONFIG(2) = getFormItemInt(F("PRef"));
      success    = true;
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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P077_data_struct());
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P077_data) {
        return success;
      }

      if (PCONFIG(0) == 0) { PCONFIG(0) = CSE_UREF_PULSE; }

      if (PCONFIG(1) == 0) { PCONFIG(1) = CSE_IREF_PULSE; }

      if (PCONFIG(2) == 0) { PCONFIG(2) = CSE_PREF_PULSE; }

      if (P077_data->init(port, serial_rx, serial_tx, 4800, static_cast<uint8_t>(SERIAL_8E1))) {
        success = true;
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
      }

      break;
    }

    case PLUGIN_READ: {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG_DEV, F("CSE: plugin read"));
      # endif // ifndef BUILD_NO_DEBUG

      // Variables set in PLUGIN_SERIAL_IN/PLUGIN_TEN_PER_SECOND as soon as there are new values!
      success = true;
      break;
    }

    case PLUGIN_SERIAL_IN:      // When using HWSerial0
    case PLUGIN_TEN_PER_SECOND: // When using other than HWSerial0
    {
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      if (((ESPEasySerialPort::serial0 == port) && (PLUGIN_TEN_PER_SECOND == function)) || // Negative checks...
          ((ESPEasySerialPort::serial0 != port) && (PLUGIN_SERIAL_IN == function))) {
        return success;
      }
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        success = true;

        /* ONLINE CHECKSUMMING by Bartłomiej Zimoń */
        if (P077_data->processSerialData()) {
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("CSE: packet found"));
          # endif // ifndef BUILD_NO_DEBUG

          if (CseReceived(event)) {
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
              String log = F("CSE: adjustment ");
              log += P077_data->adjustment;
              addLogMove(LOG_LEVEL_DEBUG_DEV, log);
              log  = F("CSE: voltage_cycle ");
              log += P077_data->voltage_cycle;
              addLogMove(LOG_LEVEL_DEBUG_DEV, log);
              log  = F("CSE: current_cycle ");
              log += P077_data->current_cycle;
              addLogMove(LOG_LEVEL_DEBUG_DEV, log);
              log  = F("CSE: power_cycle ");
              log += P077_data->power_cycle;
              addLogMove(LOG_LEVEL_DEBUG_DEV, log);
              log  = F("CSE: cf_pulses ");
              log += P077_data->cf_pulses;
              addLogMove(LOG_LEVEL_DEBUG_DEV, log);
            }

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              String log = F("CSE voltage: ");
              log += P077_data->energy_voltage;
              addLogMove(LOG_LEVEL_DEBUG, log);
              log  = F("CSE power: ");
              log += P077_data->energy_power;
              addLogMove(LOG_LEVEL_DEBUG, log);
              log  = F("CSE current: ");
              log += P077_data->energy_current;
              addLogMove(LOG_LEVEL_DEBUG, log);
              log  = F("CSE pulses: ");
              log += P077_data->cf_pulses;
              addLogMove(LOG_LEVEL_DEBUG, log);
            }
            # endif // ifndef BUILD_NO_DEBUG
          }

          // new packet received, update values
          UserVar[event->BaseVarIndex]     = P077_data->energy_voltage;
          UserVar[event->BaseVarIndex + 1] = P077_data->energy_power;
          UserVar[event->BaseVarIndex + 2] = P077_data->energy_current;
          UserVar[event->BaseVarIndex + 3] = P077_data->cf_pulses;

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("CSE: time ");
            log += P077_data->t_max;
            log += '/';
            log += P077_data->t_pkt;
            log += '/';
            log += P077_data->t_all;
            addLogMove(LOG_LEVEL_DEBUG, log);
            log  = F("CSE: bytes ");
            log += P077_data->count_bytes;
            log += '/';
            log += P077_data->count_max;
            log += '/';
            log += P077_data->serial_Available();
            addLogMove(LOG_LEVEL_DEBUG, log);
            log  = F("CSE: nr ");
            log += P077_data->count_pkt;
            addLogMove(LOG_LEVEL_DEBUG, log);
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

bool CseReceived(struct EventStruct *event) {
  P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P077_data) {
    return false;
  }

  if (!P077_data->processCseReceived(event)) {
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("CSE: Abnormal hardware"));
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }
  return true;
}

#endif // USES_P077
