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

boolean Plugin_077_init = false;

#define CSE_NOT_CALIBRATED          0xAA
#define CSE_PULSES_NOT_INITIALIZED  -1
#define CSE_PREF                    1000
#define CSE_UREF                    100
#define HLW_PREF_PULSE         12530        // was 4975us = 201Hz = 1000W
#define HLW_UREF_PULSE         1950         // was 1666us = 600Hz = 220V
#define HLW_IREF_PULSE         3500         // was 1666us = 600Hz = 4.545A

/*
   unsigned long energy_power_calibration = HLW_PREF_PULSE;
   unsigned long energy_voltage_calibration = HLW_UREF_PULSE;
   unsigned long energy_current_calibration = HLW_IREF_PULSE;
*/

uint8_t cse_receive_flag = 0;

long voltage_cycle = 0;
long current_cycle = 0;
long power_cycle = 0;
long power_cycle_first = 0;
long cf_pulses = 0;
long cf_pulses_last_time = CSE_PULSES_NOT_INITIALIZED;
long cf_frequency = 0;
uint8_t serial_in_buffer[32];
uint8_t P077_checksum = 0;
float energy_voltage = 0;         // 123.1 V
float energy_current = 0;         // 123.123 A
float energy_power = 0;           // 123.1 W

//stats
long P077_t_max = 0, P077_t_all = 0, P077_t_pkt=0, P077_t_pkt_tmp = 0;
uint16_t P077_count_bytes = 0, P077_count_max = 0, P077_count_pkt = 0;

boolean Plugin_077(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_077;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
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

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_077);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_077));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_077));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_077));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_077));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        // No pins selectable, all hard coded
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNumericBox(F("U Ref"), F("p077_URef"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        addUnit(F("uSec"));

        addFormNumericBox(F("I Ref"), F("p077_IRef"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        addUnit(F("uSec"));

        addFormNumericBox(F("P Ref"), F("p077_PRef"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        addUnit(F("uSec"));
        addFormNote(F("Use 0 to read values stored on chip / default values"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p077_URef"));;
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p077_IRef"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p077_PRef"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_077_init = true;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 0) Settings.TaskDevicePluginConfig[event->TaskIndex][0] = HLW_UREF_PULSE;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 0) Settings.TaskDevicePluginConfig[event->TaskIndex][1] = HLW_IREF_PULSE;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][2] == 0) Settings.TaskDevicePluginConfig[event->TaskIndex][2] = HLW_PREF_PULSE;

        Settings.UseSerial = true; // Enable Serial port
        disableSerialLog(); // disable logging on serial port (used for CSE7766 communication)
        Settings.BaudRate = 4800; // set BaudRate for CSE7766
        Serial.flush();
        Serial.begin(Settings.BaudRate,SERIAL_8E1);
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
              if (cf_pulses < cf_pulses_last_time) {  // Rolled over after 65535 pulses
                cf_frequency = (65536 - cf_pulses_last_time) + cf_pulses;
              } else {
                cf_frequency = cf_pulses - cf_pulses_last_time;
              }
              if (cf_frequency)  {
                cf_pulses_last_time = cf_pulses;
       //           energy_kWhtoday_delta += (cf_frequency * energy_power_calibration) / 36;
       //           EnergyUpdateToday();
              }
            }
            success = true;
            break;
          }
    */

    case PLUGIN_READ:
      {
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


    case PLUGIN_SERIAL_IN:
      {
        if (Plugin_077_init) {
          success = true;
          /* ONLINE CHECKSUMMING by Bartłomiej Zimoń */
          bool P077_found = false;
          long t_start = millis();
          while (Serial.available()>0 && !P077_found){ // if we have data try to find good checksum
            uint8_t serial_in_byte = Serial.read();
            P077_count_bytes++;
            P077_checksum -= serial_in_buffer[2]; // substract from checksum data to be removed
            memmove(serial_in_buffer,serial_in_buffer+1,sizeof(serial_in_buffer)-1); // scroll buffer
            serial_in_buffer[25] = serial_in_byte; // add new data
            P077_checksum += serial_in_buffer[22]; // add online checksum
            if (P077_checksum == serial_in_buffer[23] && serial_in_buffer[1] == 0x5A){
              P077_found = true;
              P077_count_pkt++;
              addLog(LOG_LEVEL_DEBUG, F("CSE: packet found"));
              CseReceived(event);

              // new packet received, update values
              UserVar[event->BaseVarIndex] = energy_voltage;
              UserVar[event->BaseVarIndex + 1] = energy_power;
              UserVar[event->BaseVarIndex + 2] = energy_current;
              UserVar[event->BaseVarIndex + 3] = cf_pulses;
            }
          }

          long t_diff = millis()-t_start;
          P077_t_all += t_diff;

          if (P077_count_pkt > 10) // bypass first 10 pkts
            P077_t_max = max(P077_t_max, t_diff);

          if (P077_found) {
            P077_count_max = max(P077_count_max, P077_count_bytes);
            P077_t_pkt = t_start - P077_t_pkt_tmp;
            P077_t_pkt_tmp = t_start;

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              String log = F("CSE: time ");
              log += P077_t_max;
              log += '/';
              log += P077_t_pkt;
              log += '/';
              log += P077_t_all;
              addLog(LOG_LEVEL_DEBUG, log);
              log = F("CSE: bytes ");
              log += P077_count_bytes;
              log += '/';
              log += P077_count_max;
              log += '/';
              log += Serial.available();
              addLog(LOG_LEVEL_DEBUG, log);
              log = F("CSE: nr ");
              log += P077_count_pkt;
              addLog(LOG_LEVEL_DEBUG, log);
            }

            P077_t_all = 0;
            P077_count_bytes = 0;
          }
        }
        break;
      }
  }
  return success;
}

void CseReceived(struct EventStruct *event)
{
  uint8_t header = serial_in_buffer[0];
  if ((header & 0xFC) == 0xFC) {
    addLog(LOG_LEVEL_DEBUG, F("CSE: Abnormal hardware"));
    return;
  }


  // Get chip calibration data (coefficients) and use as initial defaults
  if (HLW_UREF_PULSE == Settings.TaskDevicePluginConfig[event->TaskIndex][0]) {
    long voltage_coefficient = 191200; // uSec
    if (CSE_NOT_CALIBRATED != header) {
      voltage_coefficient = serial_in_buffer[2] << 16 | serial_in_buffer[3] << 8 | serial_in_buffer[4];
    }
    Settings.TaskDevicePluginConfig[event->TaskIndex][0] = voltage_coefficient / CSE_UREF;
  }
  if (HLW_IREF_PULSE == Settings.TaskDevicePluginConfig[event->TaskIndex][1]) {
    long current_coefficient = 16140; // uSec
    if (CSE_NOT_CALIBRATED != header) {
      current_coefficient = serial_in_buffer[8] << 16 | serial_in_buffer[9] << 8 | serial_in_buffer[10];
    }
    Settings.TaskDevicePluginConfig[event->TaskIndex][1] = current_coefficient;
  }
  if (HLW_PREF_PULSE == Settings.TaskDevicePluginConfig[event->TaskIndex][2]) {
    long power_coefficient = 5364000; // uSec
    if (CSE_NOT_CALIBRATED != header) {
      power_coefficient = serial_in_buffer[14] << 16 | serial_in_buffer[15] << 8 | serial_in_buffer[16];
    }
    Settings.TaskDevicePluginConfig[event->TaskIndex][2] = power_coefficient / CSE_PREF;
  }


  uint8_t adjustement = serial_in_buffer[20];
  voltage_cycle = serial_in_buffer[5] << 16 | serial_in_buffer[6] << 8 | serial_in_buffer[7];
  current_cycle = serial_in_buffer[11] << 16 | serial_in_buffer[12] << 8 | serial_in_buffer[13];
  power_cycle = serial_in_buffer[17] << 16 | serial_in_buffer[18] << 8 | serial_in_buffer[19];
  cf_pulses = serial_in_buffer[21] << 8 | serial_in_buffer[22];

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("CSE: adjustement ");
    log += adjustement;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
    log = F("CSE: voltage_cycle ");
    log += voltage_cycle;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
    log = F("CSE: current_cycle ");
    log += current_cycle;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
    log = F("CSE: power_cycle ");
    log += power_cycle;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
    log = F("CSE: cf_pulses ");
    log += cf_pulses;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }

  //  if (energy_power_on) {  // Powered on

  if (adjustement & 0x40) { // Voltage valid
    energy_voltage = (float)(Settings.TaskDevicePluginConfig[event->TaskIndex][0] * CSE_UREF) / (float)voltage_cycle;
  }
  if (adjustement & 0x10) { // Power valid
    if ((header & 0xF2) == 0xF2) { // Power cycle exceeds range
      energy_power = 0;
    } else {
      if (0 == power_cycle_first) power_cycle_first = power_cycle; // Skip first incomplete power_cycle
      if (power_cycle_first != power_cycle) {
        power_cycle_first = -1;
        energy_power = (float)(Settings.TaskDevicePluginConfig[event->TaskIndex][2] * CSE_PREF) / (float)power_cycle;
      } else {
        energy_power = 0;
      }
    }
  } else {
    power_cycle_first = 0;
    energy_power = 0; // Powered on but no load
  }
  if (adjustement & 0x20) { // Current valid
    if (0 == energy_power) {
      energy_current = 0;
    } else {
      energy_current = (float)Settings.TaskDevicePluginConfig[event->TaskIndex][1] / (float)current_cycle;
    }
  }

  // } else {  // Powered off
  //    power_cycle_first = 0;
  //    energy_voltage = 0;
  //    energy_power = 0;
  //    energy_current = 0;
  //  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("CSE voltage: ");
    log += energy_voltage;
    addLog(LOG_LEVEL_DEBUG, log);
    log = F("CSE power: ");
    log += energy_power;
    addLog(LOG_LEVEL_DEBUG, log);
    log = F("CSE current: ");
    log += energy_current;
    addLog(LOG_LEVEL_DEBUG, log);
    log = F("CSE pulses: ");
    log += cf_pulses;
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

#endif // USES_P077
