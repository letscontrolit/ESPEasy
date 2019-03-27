#ifdef USES_P052
//#######################################################################################################
//############################# Plugin 052: Senseair CO2 Sensors ########################################
//#######################################################################################################
/*
  Plugin originally written by: Daniel Tedenljung
  info__AT__tedenljungconsulting.com
  Rewritten by: Mikael Trieb mikael__AT__triebconsulting.se

  This plugin reads available values of Senseair Co2 Sensors.
  Datasheet can be found here:
  S8: http://www.senseair.com/products/oem-modules/senseair-s8/
  K30: http://www.senseair.com/products/oem-modules/k30/
  K70/tSENSE: http://www.senseair.com/products/wall-mount/tsense/

  Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    Use 1kOhm in serie on datapins!
*/

#define PLUGIN_052
#define PLUGIN_ID_052 52
#define PLUGIN_NAME_052 "Gases - CO2 Senseair"
#define PLUGIN_VALUENAME1_052 ""

#define P052_MEASUREMENT_INTERVAL 60000L

// For layout and status flags in RAM/EEPROM, see document
// "CO2-Engine-BLG_ELG configuration guide Rev 1_02.docx"

// RAM layout
#define P052_RAM_ADDR_ERROR_STATUS 0x1E      // U8 (error flags)
#define P052_RAM_ADDR_METER_STATUS 0x1D      // U8 (status flags)
#define P052_RAM_ADDR_ALARM_STATUS 0x1C      // U8 (alarm flags)
#define P052_RAM_ADDR_CO2 0x08               // S16 BLG: x.xxx%  ELG: x ppm
#define P052_RAM_ADDR_SPACE_TEMPERATURE 0x12 // S16 x.xx °C
#define P052_RAM_ADDR_RELATIVE_HUMIDITY 0x14 // S16 x.xx %
#define P052_RAM_ADDR_MIXING_RATIO 0x16      // S16 x.xx g/kg
#define P052_RAM_ADDR_HR1 0x40               // U16
#define P052_RAM_ADDR_HR2 0x42               // U16
#define P052_RAM_ADDR_ANIN4 0x69             // U16 x.xxx volt
#define P052_RAM_ADDR_RTC 0x65 // U32 x seconds   (virtual real time clock)
#define P052_RAM_ADDR_SCR 0x60 // U8 (special control register)

// EEPROM layout
#define P052_EEPROM_ADDR_METERCONTROL 0x03    // U8
#define P052_EEPROM_ADDR_METERCONFIG 0x06     // U16
#define P052_EEPROM_ADDR_ABC_PERIOD 0x40      // U16 ABC period in hours
#define P052_EEPROM_ADDR_HEARTBEATPERIOD 0xA2 // U8 Period in seconds
#define P052_EEPROM_ADDR_PUMPPERIOD 0xA3      // U8 Period in seconds
#define P052_EEPROM_ADDR_MEASUREMENT_SLEEP_PERIOD  0xB0 // U24 Measurement period (unit = seconds)
#define P052_EEPROM_ADDR_LOGGER_STRUCTURE_ADDRESS  0x200 // 16b Described in “BLG_ELG Logger Structure”

// SCR (Special Control Register) commands
#define P052_SCR_FORCE_START_MEASUREMENT 0x30
#define P052_SCR_FORCE_STOP_MEASUREMENT 0x31
#define P052_SCR_RESTART_LOGGER 0x32      // (logger data erased)
#define P052_SCR_REINITIALIZE_LOGGER 0x33 // (logger data unaffected)
#define P052_SCR_WRITE_TIMESTAMP_TO_LOGGER 0x34
#define P052_SCR_SINGLE_MEASUREMENT 0x35

#define P052_IR_METERSTATUS 0
#define P052_IR_ALARMSTATUS 1
#define P052_IR_OUTPUTSTATUS 2
#define P052_IR_SPACE_CO2 3

#define P052_HR_ACK_REG 0
#define P052_HR_SPACE_CO2 3
#define P052_HR_ABC_PERIOD 31

#define P052_MODBUS_SLAVE_ADDRESS 0xFE // Modbus "any address"

#include <ESPeasySerial.h>


struct P052_data_struct : public PluginTaskData_base {
  P052_data_struct() {}

  ~P052_data_struct() { reset(); }

  void reset() {
    modbus.reset();
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx) {
    return modbus.init(serial_rx, serial_tx, 9600, P052_MODBUS_SLAVE_ADDRESS);
  }

  bool isInitialized() const {
    return modbus.isInitialized();
  }

  ModbusRTU_struct modbus;
};

unsigned int _plugin_052_last_measurement = 0;

boolean Plugin_052(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {

  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_052;
    Device[deviceCount].Type = DEVICE_TYPE_DUAL;
    Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 1;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME: {
    string = F(PLUGIN_NAME_052);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES: {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
             PSTR(PLUGIN_VALUENAME1_052));
    break;
  }

  case PLUGIN_GET_DEVICEGPIONAMES: {
    serialHelper_getGpioNames(event);
    break;
  }

  case PLUGIN_WRITE: {
    String cmd = parseString(string, 1);
    String param1 = parseString(string, 2);

    if (cmd.equalsIgnoreCase(F("senseair_setrelay"))) {
      int par1;
      if (validIntFromString(param1, par1)) {
        if (par1 == 0 || par1 == 1 || par1 == -1) {
          short relaystatus = 0; // 0x3FFF represents 100% output.
          //  Refer to sensor model’s specification for voltage at 100% output.
          switch (par1) {
          case 0:
            relaystatus = 0;
            break;
          case 1:
            relaystatus = 0x3FFF;
            break;
          default:
            relaystatus = 0x7FFF;
            break;
          }
          P052_data_struct *P052_data =
              static_cast<P052_data_struct *>(getPluginTaskData(event->TaskIndex));
          if (nullptr != P052_data && P052_data->isInitialized()) {
            P052_data->modbus.writeSingleRegister(0x18, relaystatus);
            addLog(LOG_LEVEL_INFO, String(F("Senseair command: relay=")) + param1);
          }
        }
      }
      success = true;
    }

    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    if (cmd.equalsIgnoreCase(F("senseair_setABCperiod")))
    {
      if (param1.toInt() >= 0) {
        Plugin_052_setABCperiod(param1.toInt());
        addLog(LOG_LEVEL_INFO, String(F("Senseair command: ABCperiod=")) +
    param1);
      }
      success = true;
    }
    */

    break;
  }

  case PLUGIN_WEBFORM_LOAD: {
    serialHelper_webformLoad(event);
    byte choiceSensor = PCONFIG(0);

    String optionsSensor[7] = {F("Error Status"), F("Carbon Dioxide"),
                               F("Temperature"),  F("Humidity"),
                               F("Relay Status"), F("Temperature Adjustment"),
                               F("ABC period")};
    addFormSelector(F("Sensor"), F("p052_sensor"), 7, optionsSensor, NULL,
                    choiceSensor);

    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    byte choiceABCperiod = PCONFIG(1);
    String optionsABCperiod[9] = { F("disable"), F("1 h"), F("12 h"), F("1
    day"), F("2 days"), F("4 days"), F("7 days"), F("14 days"), F("30 days") };
    addFormSelector(F("ABC period"), F("p052_ABC_period"), 9, optionsABCperiod,
    NULL, choiceABCperiod);
    */

    P052_data_struct *P052_data =
        static_cast<P052_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P052_data && P052_data->isInitialized()) {
      String detectedString = P052_data->modbus.detected_device_description;
      if (detectedString.length() > 0) {
        addFormNote(detectedString);
      }
      addRowLabel(F("Checksum (pass/fail)"));
      uint32_t reads_pass, reads_crc_failed;
      P052_data->modbus.getStatistics(reads_pass, reads_crc_failed);
      String chksumStats;
      chksumStats = reads_pass;
      chksumStats += '/';
      chksumStats += reads_crc_failed;
      addHtml(chksumStats);
    }
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    serialHelper_webformSave(event);
    PCONFIG(0) = getFormItemInt(F("p052_sensor"));
    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    PCONFIG(1) = getFormItemInt(F("p052_ABC_period"));
    */

    success = true;
    break;
  }

  case PLUGIN_INIT: {
    const int16_t serial_rx = CONFIG_PIN1;
    const int16_t serial_tx = CONFIG_PIN2;
    initPluginTaskData(event->TaskIndex, new P052_data_struct());
    P052_data_struct *P052_data =
        static_cast<P052_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr == P052_data) {
      return success;
    }
    if (P052_data->init(serial_rx, serial_tx)) {
      /*
      // ABC functionality disabled for now, due to a bug in the firmware.
      // See https://github.com/letscontrolit/ESPEasy/issues/759
      const int periodInHours[9] = {0, 1, 12, (24*1), (24*2), (24*4), (24*7),
      (24*14), (24*30) };
      byte choiceABCperiod = PCONFIG(1);

      Plugin_052_setABCperiod(periodInHours[choiceABCperiod]);
      */

      success = true;
    } else {
      clearPluginTaskData(event->TaskIndex);
    }
    break;
  }

  case PLUGIN_EXIT: {
    clearPluginTaskData(event->TaskIndex);
    success = true;
    break;
  }

  case PLUGIN_READ: {
    P052_data_struct *P052_data =
        static_cast<P052_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P052_data && P052_data->isInitialized()) {
      String log = F("Senseair: ");
      switch (PCONFIG(0)) {
      case 0: {
        int errorWord = P052_data->modbus.readInputRegister(0x00);
        for (size_t i = 0; i < 9; i++) {
          if (bitRead(errorWord, i)) {
            UserVar[event->BaseVarIndex] = i;
            log += F("error code = ");
            log += i;
            break;
          }
        }

        UserVar[event->BaseVarIndex] = -1;
        log += F("error code = ");
        log += -1;
        break;
      }
      case 1: {
        int co2 = P052_data->modbus.readInputRegister(0x03);
        UserVar[event->BaseVarIndex] = co2;
        log += F("co2 = ");
        log += co2;
        break;
      }
      case 2: {
        int temperatureX100 = P052_data->modbus.readInputRegister(0x04);
        float temperature = static_cast<float>(temperatureX100) / 100.0;
        UserVar[event->BaseVarIndex] = (float)temperature;
        log += F("temperature = ");
        log += (float)temperature;
        break;
      }
      case 3: {
        int rhX100 = P052_data->modbus.readInputRegister(0x05);
        float rh = static_cast<float>(rhX100) / 100.0;
        UserVar[event->BaseVarIndex] = rh;
        log += F("humidity = ");
        log += rh;
        break;
      }
      case 4: {
        int status = P052_data->modbus.readInputRegister(0x1C);
        int relayStatus = (status >> 8) & 0x1;
        UserVar[event->BaseVarIndex] = relayStatus;
        log += F("relay status = ");
        log += relayStatus;
        break;
      }
      case 5: {
        int temperatureAdjustment = P052_data->modbus.readInputRegister(0x0A);
        UserVar[event->BaseVarIndex] = temperatureAdjustment;
        log += F("temperature adjustment = ");
        log += temperatureAdjustment;
        break;
      }
      }
      addLog(LOG_LEVEL_INFO, log);

      success = true;
      break;
    }
    break;
  }
  }
  return success;
}

/*
bool getBitOfInt(int reg, int pos) {
  // Create a mask
  int mask = 0x01 << pos;

  // Mask the status register
  int masked_register = mask & reg;

  // Shift the result of masked register back to position 0
  int result = masked_register >> pos;
  return (result == 1);
}

bool Plugin_052_check_error_status() {
  byte error_status = P052_data->modbus.read_RAM_EEPROM(P052_CMD_READ_RAM,
                                                 P052_RAM_ADDR_ERROR_STATUS, 1);
  if (error_status == 0)
    return true;
  String log = F("P052 Error status:");
  for (int i = 0; i < 8; ++i) {
    if (getBitOfInt(error_status, i)) {
      log += F(" (");
      switch (i) {
      case 0:
        log += F("fatal");
        break;
      case 1:
        log += F("CO2");
        break;
      case 2:
        log += F("T/H comm");
        break;
      case 4:
        log += F("detector temp range");
        break;
      case 5:
        log += F("CO2 range");
        break;
      case 6:
        log += F("mem err.");
        break;
      case 7:
        log += F("room temp range");
        break;
      default:
        log += F("unknown");
        break;
      }
      log += F(")");
    }
  }
  addLog(LOG_LEVEL_INFO, log);
  return false;
}

int Plugin_052_readCo2_from_RAM(void) {
  bool valid_measurement = Plugin_052_prepare_single_measurement_from_RAM();
  short co2 =
      P052_data->modbus.read_RAM_EEPROM(P052_CMD_READ_RAM, P052_RAM_ADDR_CO2, 2);
  short temperature = P052_data->modbus.read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_SPACE_TEMPERATURE, 2);
  short humidity = P052_data->modbus.read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_RELATIVE_HUMIDITY, 2);
  String log = F("P052: ");
  log += F("CO2: ");
  log += co2;
  log += F(" ppm Temp: ");
  log += (float)temperature / 100.0;
  log += F(" C Hum: ");
  log += (float)humidity / 100.0;
  log += F("%");
  if (!valid_measurement)
    log += F(" (old)");
  addLog(LOG_LEVEL_INFO, log);
  return co2;
}

bool Plugin_052_measurement_active() {
  unsigned int meter_status = P052_data->modbus.read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_METER_STATUS, 1);
  // Meter Status bit 5 indicates single cycle measurement active
  return getBitOfInt(meter_status, 5);
}

// Perform a single measurement.
// return value indicates a successful measurement update.
bool Plugin_052_prepare_single_measurement_from_RAM() {
  Plugin_052_check_error_status();
  if (timeOutReached(_plugin_052_last_measurement +
                     P052_MEASUREMENT_INTERVAL)) {
    // Last measurement taken is still valid.
    return true;
  }
  int retry_count = 2;
  addLog(LOG_LEVEL_INFO, F("P052: Start perform measurement"));
  while (!Plugin_052_measurement_active() && retry_count > 0) {
    // Trigger new measurement and make sure it is set active.
    --retry_count;
    addLog(LOG_LEVEL_INFO, F("P052: Write to SCR: perform measurement"));
    Plugin_052_writeSpecialCommandRegister(P052_SCR_SINGLE_MEASUREMENT);
    delay(50);
  }
  if (!Plugin_052_measurement_active()) {
    // Could not start measurement.
    addLog(LOG_LEVEL_INFO, F("P052: Could not start single measurement"));
    return false;
  }
  retry_count = 30;
  while (retry_count > 0) {
    --retry_count;
    if (retry_count < 14) {
      // Just wait for 16 seconds.
      if (!Plugin_052_measurement_active()) {
        _plugin_052_last_measurement = millis();
        String log = F("P052: Measurement complete after ");
        log += (30 - retry_count);
        addLog(LOG_LEVEL_INFO, log);
        return true;
      }
    }
    delay(1000);
  }
  return false;
}




int Plugin_052_readABCperiod(void) {
  return P052_data->modbus.readHoldingRegister(0x1F);
}

int Plugin_052_readModbusAddress(void) {
  return P052_data->modbus.readHoldingRegister(63); // HR64 MAC address Modbus address, valid range 1 - 253
}
*/

#endif // USES_P052
