#include "_Plugin_Helper.h"
#ifdef USES_P077
//#######################################################################################################
//###################### Plugin 077: CSE7766 - Energy (Sonoff S31 and Sonoff Pow R2) ####################
//#######################################################################################################
//###################################### stefan@clumsy.ch      ##########################################
//#######################################################################################################



#define PLUGIN_077
#define PLUGIN_ID_077         77
#ifdef PLUGIN_SET_SONOFF_POW
  #define PLUGIN_NAME_077       "Energy (AC) - CSE7766 (POW r2) [TESTING]"
#else
  #define PLUGIN_NAME_077       "Energy (AC) - CSE7766 [TESTING]"
#endif
#define PLUGIN_VALUENAME1_077 "Voltage"
#define PLUGIN_VALUENAME2_077 "Power"
#define PLUGIN_VALUENAME3_077 "Current"
#define PLUGIN_VALUENAME4_077 "Pulses"

#define CSE_NOT_CALIBRATED          0xAA
#define CSE_PULSES_NOT_INITIALIZED  -1
#define CSE_PREF                    1000
#define CSE_UREF                    100
#define HLW_PREF_PULSE              12530        // was 4975us = 201Hz = 1000W
#define HLW_UREF_PULSE              1950         // was 1666us = 600Hz = 220V
#define HLW_IREF_PULSE              3500         // was 1666us = 600Hz = 4.545A

/*
   unsigned long energy_power_calibration = HLW_PREF_PULSE;
   unsigned long energy_voltage_calibration = HLW_UREF_PULSE;
   unsigned long energy_current_calibration = HLW_IREF_PULSE;
*/

struct P077_data_struct : public PluginTaskData_base {

  bool processCseReceived(struct EventStruct *event) {
    uint8_t header = serial_in_buffer[0];
    if ((header & 0xFC) == 0xFC) {
      //  Abnormal hardware
      return false;
    }

    // Get chip calibration data (coefficients) and use as initial defaults
    if (HLW_UREF_PULSE == PCONFIG(0)) {
      long voltage_coefficient = 191200; // uSec
      if (CSE_NOT_CALIBRATED != header) {
        voltage_coefficient = serial_in_buffer[2] << 16 |
                              serial_in_buffer[3] << 8 | serial_in_buffer[4];
      }
      PCONFIG(0) = voltage_coefficient / CSE_UREF;
    }
    if (HLW_IREF_PULSE == PCONFIG(1)) {
      long current_coefficient = 16140; // uSec
      if (CSE_NOT_CALIBRATED != header) {
        current_coefficient = serial_in_buffer[8] << 16 |
                              serial_in_buffer[9] << 8 | serial_in_buffer[10];
      }
      PCONFIG(1) = current_coefficient;
    }
    if (HLW_PREF_PULSE == PCONFIG(2)) {
      long power_coefficient = 5364000; // uSec
      if (CSE_NOT_CALIBRATED != header) {
        power_coefficient = serial_in_buffer[14] << 16 |
                            serial_in_buffer[15] << 8 | serial_in_buffer[16];
      }
      PCONFIG(2) = power_coefficient / CSE_PREF;
    }

    adjustment = serial_in_buffer[20];
    voltage_cycle = serial_in_buffer[5] << 16 | serial_in_buffer[6] << 8 |
                    serial_in_buffer[7];
    current_cycle = serial_in_buffer[11] << 16 | serial_in_buffer[12] << 8 |
                    serial_in_buffer[13];
    power_cycle = serial_in_buffer[17] << 16 | serial_in_buffer[18] << 8 |
                  serial_in_buffer[19];
    cf_pulses = serial_in_buffer[21] << 8 | serial_in_buffer[22];

    //  if (energy_power_on) {  // Powered on

    if (adjustment & 0x40) { // Voltage valid
      energy_voltage = (float)(PCONFIG(0) * CSE_UREF) / (float)voltage_cycle;
    }
    if (adjustment & 0x10) {        // Power valid
      if ((header & 0xF2) == 0xF2) { // Power cycle exceeds range
        energy_power = 0;
      } else {
        if (0 == power_cycle_first)
          power_cycle_first = power_cycle; // Skip first incomplete power_cycle
        if (power_cycle_first != power_cycle) {
          power_cycle_first = -1;
          energy_power = (float)(PCONFIG(2) * CSE_PREF) / (float)power_cycle;
        } else {
          energy_power = 0;
        }
      }
    } else {
      power_cycle_first = 0;
      energy_power = 0; // Powered on but no load
    }
    if (adjustment & 0x20) { // Current valid
      if (0 == energy_power) {
        energy_current = 0;
      } else {
        energy_current = (float)PCONFIG(1) / (float)current_cycle;
      }
    }

    // } else {  // Powered off
    //    power_cycle_first = 0;
    //    energy_voltage = 0;
    //    energy_power = 0;
    //    energy_current = 0;
    //  }



    return true;
  }

  bool processSerialData() {
    long t_start = millis();
    bool found = false;
    while (Serial.available() > 0 && !found) {
      uint8_t serial_in_byte = Serial.read();
      count_bytes++;
      checksum -= serial_in_buffer[2]; // substract from checksum data to be removed
      memmove(serial_in_buffer, serial_in_buffer + 1,
              sizeof(serial_in_buffer) - 1); // scroll buffer
      serial_in_buffer[25] = serial_in_byte; // add new data
      checksum += serial_in_buffer[22]; // add online checksum
      if (checksum == serial_in_buffer[23] &&
          serial_in_buffer[1] == 0x5A) {
        count_pkt++;
        found = true;
      }
    }
    long t_diff = timePassedSince(t_start);
    t_all += t_diff;

    if (count_pkt > 10) { // bypass first 10 pkts
      t_max = max(t_max, t_diff);
    }
    if (found) {
      count_max = max(count_max, count_bytes);
      t_pkt = t_start - t_pkt_tmp;
      t_pkt_tmp = t_start;

    }

    return found;
  }

  //  uint8_t cse_receive_flag = 0;

  uint8_t serial_in_buffer[32];
  long voltage_cycle = 0;
  long current_cycle = 0;
  long power_cycle = 0;
  long power_cycle_first = 0;
  long cf_pulses = 0;
  long cf_pulses_last_time = CSE_PULSES_NOT_INITIALIZED;
  long cf_frequency = 0;
  float energy_voltage = 0; // 123.1 V
  float energy_current = 0; // 123.123 A
  float energy_power = 0;   // 123.1 W

  // stats
  long t_max = 0, t_all = 0, t_pkt = 0, t_pkt_tmp = 0;
  uint16_t count_bytes = 0, count_max = 0, count_pkt = 0;
  uint8_t checksum = 0, adjustment = 0;
};



boolean Plugin_077(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_077;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].TimerOptional = true;
    Device[deviceCount].GlobalSyncOption = true;
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
    ;
    PCONFIG(1) = getFormItemInt(F("p077_IRef"));
    PCONFIG(2) = getFormItemInt(F("p077_PRef"));
    success = true;
    break;
  }

  case PLUGIN_INIT: {
    initPluginTaskData(event->TaskIndex, new (std::nothrow) P077_data_struct());
    if (PCONFIG(0) == 0) PCONFIG(0) = HLW_UREF_PULSE;
    if (PCONFIG(1) == 0) PCONFIG(1) = HLW_IREF_PULSE;
    if (PCONFIG(2) == 0) PCONFIG(2) = HLW_PREF_PULSE;

    Settings.UseSerial = true; // Enable Serial port
    disableSerialLog(); // disable logging on serial port (used for CSE7766
                        // communication)
    Settings.BaudRate = 4800; // set BaudRate for CSE7766
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
    addLog(LOG_LEVEL_DEBUG_DEV, F("CSE: plugin read"));
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
    P077_data_struct* P077_data = static_cast<P077_data_struct*>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P077_data) {
      success = true;
      /* ONLINE CHECKSUMMING by Bartłomiej Zimoń */
      if (P077_data->processSerialData()) {
        addLog(LOG_LEVEL_DEBUG, F("CSE: packet found"));
        if (CseReceived(event)) {
          if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
            String log = F("CSE: adjustment ");
            log += P077_data->adjustment;
            addLog(LOG_LEVEL_DEBUG_DEV, log);
            log = F("CSE: voltage_cycle ");
            log += P077_data->voltage_cycle;
            addLog(LOG_LEVEL_DEBUG_DEV, log);
            log = F("CSE: current_cycle ");
            log += P077_data->current_cycle;
            addLog(LOG_LEVEL_DEBUG_DEV, log);
            log = F("CSE: power_cycle ");
            log += P077_data->power_cycle;
            addLog(LOG_LEVEL_DEBUG_DEV, log);
            log = F("CSE: cf_pulses ");
            log += P077_data->cf_pulses;
            addLog(LOG_LEVEL_DEBUG_DEV, log);
          }

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("CSE voltage: ");
            log += P077_data->energy_voltage;
            addLog(LOG_LEVEL_DEBUG, log);
            log = F("CSE power: ");
            log += P077_data->energy_power;
            addLog(LOG_LEVEL_DEBUG, log);
            log = F("CSE current: ");
            log += P077_data->energy_current;
            addLog(LOG_LEVEL_DEBUG, log);
            log = F("CSE pulses: ");
            log += P077_data->cf_pulses;
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }

        // new packet received, update values
        UserVar[event->BaseVarIndex] = P077_data->energy_voltage;
        UserVar[event->BaseVarIndex + 1] = P077_data->energy_power;
        UserVar[event->BaseVarIndex + 2] = P077_data->energy_current;
        UserVar[event->BaseVarIndex + 3] = P077_data->cf_pulses;


        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("CSE: time ");
          log += P077_data->t_max;
          log += '/';
          log += P077_data->t_pkt;
          log += '/';
          log += P077_data->t_all;
          addLog(LOG_LEVEL_DEBUG, log);
          log = F("CSE: bytes ");
          log += P077_data->count_bytes;
          log += '/';
          log += P077_data->count_max;
          log += '/';
          log += Serial.available();
          addLog(LOG_LEVEL_DEBUG, log);
          log = F("CSE: nr ");
          log += P077_data->count_pkt;
          addLog(LOG_LEVEL_DEBUG, log);
        }
        P077_data->t_all = 0;
        P077_data->count_bytes = 0;
      }
    }
    break;
  }
  }
  return success;
}

bool CseReceived(struct EventStruct *event) {
  P077_data_struct* P077_data = static_cast<P077_data_struct*>(getPluginTaskData(event->TaskIndex));
  if (nullptr == P077_data) {
    return false;
  }
  if (!P077_data->processCseReceived(event)) {
    addLog(LOG_LEVEL_DEBUG, F("CSE: Abnormal hardware"));
    return false;
  }
  return true;
}

#endif // USES_P077
