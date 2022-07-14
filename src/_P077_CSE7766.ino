#include "_Plugin_Helper.h"
#ifdef USES_P077

// #######################################################################################################
// ###################### Plugin 077: CSE7766 - Energy (Sonoff S31 and Sonoff Pow R2) ####################
// #######################################################################################################
// ###################################### stefan@clumsy.ch      ##########################################
// #######################################################################################################

# include "src/PluginStructs/P077_data_struct.h"

# define PLUGIN_077
# define PLUGIN_ID_077         77
# ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766 (POW r2) [TESTING]"
# else // ifdef PLUGIN_SET_SONOFF_POW
  #  define PLUGIN_NAME_077       "Energy (AC) - CSE7766 [TESTING]"
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

    case PLUGIN_GET_DEVICEGPIONAMES: {
      // No pins selectable, all hard coded
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormNumericBox(F("U Ref"), F("p077_URef"), PCONFIG(0));
      addUnit(F("uSec"));

      addFormNumericBox(F("I Ref"), F("p077_IRef"), PCONFIG(1));
      addUnit(F("uSec"));

      addFormNumericBox(F("P Ref"), F("p077_PRef"), PCONFIG(2));
      addUnit(F("uSec"));
      addFormNote(F("Use 0 to read values stored on chip / default values"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      PCONFIG(0) = getFormItemInt(F("p077_URef"));
      PCONFIG(1) = getFormItemInt(F("p077_IRef"));
      PCONFIG(2) = getFormItemInt(F("p077_PRef"));
      success    = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P077_data_struct());

      if (PCONFIG(0) == 0) { PCONFIG(0) = HLW_UREF_PULSE; }

      if (PCONFIG(1) == 0) { PCONFIG(1) = HLW_IREF_PULSE; }

      if (PCONFIG(2) == 0) { PCONFIG(2) = HLW_PREF_PULSE; }

      Settings.UseSerial = true; // Enable Serial port
      disableSerialLog();        // disable logging on serial port (used for CSE7766
                                 // communication)
      Settings.BaudRate = 4800;  // set BaudRate for CSE7766
      Serial.flush();
      Serial.begin(Settings.BaudRate, SERIAL_8E1);
      success = true;
      break;
    }

    /* currently not needed!
       case PLUGIN_TEN_PER_SECOND:
          {

            long cf_frequency = 0;

            if (CSE_PULSES_NOT_INITIALIZED == cf_pulses_last_time) {
              cf_pulses_last_time = cf_pulses;  // Init after restart
            } else {
              if (cf_pulses < cf_pulses_last_time) {  // Rolled over after 65535
       pulses
                cf_frequency = (65536 - cf_pulses_last_time) + cf_pulses;
              } else {
                cf_frequency = cf_pulses - cf_pulses_last_time;
              }
              if (cf_frequency)  {
                cf_pulses_last_time = cf_pulses;
       //           energy_kWhtoday_delta += (cf_frequency *
       energy_power_calibration) / 36;
       //           EnergyUpdateToday();
              }
            }
            success = true;
            break;
          }
     */

    case PLUGIN_READ: {
    # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG_DEV, F("CSE: plugin read"));
    # endif // ifndef BUILD_NO_DEBUG

      //        sendData(event);
      //        Variables set in PLUGIN_SERIAL_IN as soon as there are new values!
      //        UserVar[event->BaseVarIndex] = energy_voltage;
      //        UserVar[event->BaseVarIndex + 1] = energy_power;
      //        UserVar[event->BaseVarIndex + 2] = energy_current;
      //        UserVar[event->BaseVarIndex + 3] = cf_pulses;
      success = true;
      break;
    }

    case PLUGIN_SERIAL_IN: {
      P077_data_struct *P077_data = static_cast<P077_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P077_data) {
        success = true;

        /* ONLINE CHECKSUMMING by Bartłomiej Zimoń */
        if (P077_data->processSerialData()) {
        # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("CSE: packet found"));

          if (CseReceived(event)) {
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
          }
        # endif // ifndef BUILD_NO_DEBUG

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
            log += Serial.available();
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
