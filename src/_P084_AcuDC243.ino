#ifdef USES_P084
//#######################################################################################################
//############################# Plugin 084: AccuEnergy AcuDC24x
//#########################################
//#######################################################################################################
/*

  Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    GPIO Setting 3 -> DE/RE pin for MAX485
    Use 1kOhm in serie on datapins!
*/

#define PLUGIN_084
#define PLUGIN_ID_084 84
#define PLUGIN_NAME_084 "Energy - AccuEnergy AcuDC24x [TESTING]"
#define PLUGIN_VALUENAME1_084 ""

#define P084_DEV_ID PCONFIG(0)
#define P084_DEV_ID_LABEL PCONFIG_LABEL(0)
#define P084_MODEL PCONFIG(1)
#define P084_MODEL_LABEL PCONFIG_LABEL(1)
#define P084_BAUDRATE PCONFIG(2)
#define P084_BAUDRATE_LABEL PCONFIG_LABEL(2)
#define P084_QUERY1 PCONFIG(3)
#define P084_QUERY2 PCONFIG(4)
#define P084_QUERY3 PCONFIG(5)
#define P084_QUERY4 PCONFIG(6)
#define P084_DEPIN CONFIG_PIN3

#define P084_DEV_ID_DFLT 1   // Modbus communication address
#define P084_MODEL_DFLT 0    // SDM120C
#define P084_BAUDRATE_DFLT 4 // 19200 baud
#define P084_QUERY1_DFLT 0   // Voltage (V)
#define P084_QUERY2_DFLT 1   // Current (A)
#define P084_QUERY3_DFLT 2   // Power (W)
#define P084_QUERY4_DFLT 5   // Power Factor (cos-phi)

#define P084_MEASUREMENT_INTERVAL 60000L

#define P084_SENSOR_TYPE_INDEX (VARS_PER_TASK + 3)

#define P084_QUERY_V       0
#define P084_QUERY_A       1
#define P084_QUERY_W       2
#define P084_QUERY_Wh_imp  3
#define P084_QUERY_Wh_exp  4
#define P084_QUERY_Wh_tot  5
#define P084_QUERY_Wh_net  6
#define P084_QUERY_h_tot   7
#define P084_QUERY_h_load  8

#include <ESPeasySerial.h>

struct P084_data_struct : public PluginTaskData_base {
  P084_data_struct() {}

  ~P084_data_struct() { reset(); }

  void reset() { modbus.reset(); }

  bool init(const int16_t serial_rx, const int16_t serial_tx, int8_t dere_pin,
            unsigned int baudrate, uint8_t modbusAddress) {
    return modbus.init(serial_rx, serial_tx, baudrate, modbusAddress, dere_pin);
  }

  bool isInitialized() const { return modbus.isInitialized(); }

  ModbusRTU_struct modbus;
};

unsigned int _plugin_084_last_measurement = 0;

boolean Plugin_084(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {

  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_084;
    Device[deviceCount].Type =
        DEVICE_TYPE_TRIPLE; // connected through 3 datapins
    Device[deviceCount].VType = SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME: {
    string = F(PLUGIN_NAME_084);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES: {
    safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[0],
                 p084_getQueryValueString(P084_QUERY1),
                 sizeof(ExtraTaskSettings.TaskDeviceValueNames[0]));
    safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[1],
                 p084_getQueryValueString(P084_QUERY2),
                 sizeof(ExtraTaskSettings.TaskDeviceValueNames[1]));
    safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[2],
                 p084_getQueryValueString(P084_QUERY3),
                 sizeof(ExtraTaskSettings.TaskDeviceValueNames[2]));
    safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[3],
                 p084_getQueryValueString(P084_QUERY4),
                 sizeof(ExtraTaskSettings.TaskDeviceValueNames[3]));
    break;
  }

  case PLUGIN_GET_DEVICEGPIONAMES: {
    serialHelper_getGpioNames(event);
    event->String3 = formatGpioName_output_optional("DE");
    break;
  }

  case PLUGIN_SET_DEFAULTS: {
    P084_DEV_ID = P084_DEV_ID_DFLT;
    P084_MODEL = P084_MODEL_DFLT;
    P084_BAUDRATE = P084_BAUDRATE_DFLT;
    P084_QUERY1 = P084_QUERY1_DFLT;
    P084_QUERY2 = P084_QUERY2_DFLT;
    P084_QUERY3 = P084_QUERY3_DFLT;
    P084_QUERY4 = P084_QUERY4_DFLT;

    //    PCONFIG(P084_SENSOR_TYPE_INDEX) = SENSOR_TYPE_SINGLE;
    success = true;
    break;
  }

  case PLUGIN_WRITE: {
    break;
  }

  case PLUGIN_WEBFORM_LOAD: {
    serialHelper_webformLoad(event);

    String options_baudrate[6];
    for (int i = 0; i < 6; ++i) {
      options_baudrate[i] = String(p084_storageValueToBaudrate(i));
    }
    addFormNumericBox(F("Modbus Address"), P084_DEV_ID_LABEL, P084_DEV_ID, 1,
                      247);
    addFormSelector(F("Baud Rate"), P084_BAUDRATE_LABEL, 6, options_baudrate,
                    NULL, P084_BAUDRATE);

    P084_data_struct *P084_data =
        static_cast<P084_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P084_data && P084_data->isInitialized()) {
      String detectedString = P084_data->modbus.detected_device_description;
      if (detectedString.length() > 0) {
        addFormNote(detectedString);
      }
      addRowLabel(F("Checksum (pass/fail)"));
      uint32_t reads_pass, reads_crc_failed;
      P084_data->modbus.getStatistics(reads_pass, reads_crc_failed);
      String chksumStats;
      chksumStats = reads_pass;
      chksumStats += '/';
      chksumStats += reads_crc_failed;
      addHtml(chksumStats);

      addFormSubHeader(F("Calibration"));

      addFormNumericBox(F("Full Range Voltage Value"), F("p084_fr_volt"), P084_data->modbus.readHoldingRegister(0x107), 5, 9999);
      addUnit(F("V"));

      addFormNumericBox(F("Full Range Current Value"), F("p084_fr_curr"), P084_data->modbus.readHoldingRegister(0x104), 20, 50000);
      addUnit(F("A"));

      addFormNumericBox(F("Full Range Shunt Value"), F("p084_fr_shunt"), P084_data->modbus.readHoldingRegister(0x105), 50, 100);
      addUnit(F("mV"));

      addFormSubHeader(F("Logging"));

      addFormCheckBox(F("Enable data logging"), F("p084_en_log"), P084_data->modbus.readHoldingRegister(0x500));

      addRowLabel(F("Mode of data logging"));
      addHtml(String(P084_data->modbus.readHoldingRegister(0x501)));

      addFormNumericBox(F("Log Interval"), F("p084_log_int"), P084_data->modbus.readHoldingRegister(0x502), 1, 1440);
      addUnit(F("minutes"));

      addFormSubHeader(F("Logged Values"));
      p084_showValueLoadPage(P084_QUERY_Wh_imp, event);
      p084_showValueLoadPage(P084_QUERY_Wh_exp, event);
      p084_showValueLoadPage(P084_QUERY_Wh_tot, event);
      p084_showValueLoadPage(P084_QUERY_Wh_net, event);
      p084_showValueLoadPage(P084_QUERY_h_tot,  event);
      p084_showValueLoadPage(P084_QUERY_h_load, event);

      addFormCheckBox(F("Clear logged values"), F("p084_clear_log"), false);
      addFormNote(F("Will clear all logged values when checked and saved"));
    }

//    sensorTypeHelper_webformLoad_simple(event, P084_SENSOR_TYPE_INDEX);

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    serialHelper_webformSave(event);
    for (int i = 0; i < 7; ++i) {
      pconfig_webformSave(event, i);
    }
//    sensorTypeHelper_saveSensorType(event, P084_SENSOR_TYPE_INDEX);
    P084_data_struct *P084_data =
        static_cast<P084_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P084_data && P084_data->isInitialized()) {
      uint16_t log_enabled = isFormItemChecked(F("p084_en_log")) ? 1 : 0;
      P084_data->modbus.writeMultipleRegisters(0x500, log_enabled);
      delay(1);

      uint16_t log_int = getFormItemInt(F("p084_log_int"));
      P084_data->modbus.writeMultipleRegisters(0x502, log_int);
      delay(1);

      uint16_t current = getFormItemInt(F("p084_fr_curr"));
      P084_data->modbus.writeMultipleRegisters(0x104, current);
      delay(1);

      uint16_t shunt = getFormItemInt(F("p084_fr_shunt"));
      P084_data->modbus.writeMultipleRegisters(0x105, shunt);
      delay(1);

      uint16_t voltage = getFormItemInt(F("p084_fr_volt"));
      P084_data->modbus.writeMultipleRegisters(0x107, voltage);

      if (isFormItemChecked(F("p084_clear_log")))
      {
        // Clear all logged values in the meter.
        P084_data->modbus.writeMultipleRegisters(0x122, 0x0A); // Clear Energy
        P084_data->modbus.writeMultipleRegisters(0x123, 0x0A); // Clear Meter Running Hour
        P084_data->modbus.writeMultipleRegisters(0x124, 0x0A); // Clear Meter Load Hour
        P084_data->modbus.writeMultipleRegisters(0x127, 0x0A); // Clear Ah
        P084_data->modbus.writeMultipleRegisters(0x128, 0x0A); // Clear Min/Max value
        P084_data->modbus.writeMultipleRegisters(0x129, 0x0A); // Clear Data Logging
      }
    }


    success = true;
    break;
  }

  case PLUGIN_INIT: {
    const int16_t serial_rx = CONFIG_PIN1;
    const int16_t serial_tx = CONFIG_PIN2;
    initPluginTaskData(event->TaskIndex, new P084_data_struct());
    P084_data_struct *P084_data =
        static_cast<P084_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr == P084_data) {
      return success;
    }
    if (P084_data->init(serial_rx, serial_tx, P084_DEPIN,
                        p084_storageValueToBaudrate(P084_BAUDRATE),
                        P084_DEV_ID)) {
//      sensorTypeHelper_setSensorType(event, P084_SENSOR_TYPE_INDEX);

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
    P084_data_struct *P084_data =
        static_cast<P084_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr != P084_data && P084_data->isInitialized()) {
//      event->sensorType = PCONFIG(P084_SENSOR_TYPE_INDEX);

      UserVar[event->BaseVarIndex + 0] = p084_readValue(P084_QUERY_V, event); // voltage
      delay(1);
      UserVar[event->BaseVarIndex + 1] = p084_readValue(P084_QUERY_A, event); // current
      delay(1);
      UserVar[event->BaseVarIndex + 2] = p084_readValue(P084_QUERY_W, event); // power (kW => W)
      delay(1);
      UserVar[event->BaseVarIndex + 3] = p084_readValue(P084_QUERY_Wh_tot, event); // total energy

      success = true;
    }
    break;
  }
  }
  return success;
}

String p084_getQueryString(byte query) {
  switch (query) {
  case P084_QUERY_V:
    return F("Voltage (V)");
  case P084_QUERY_A:
    return F("Current (A)");
  case P084_QUERY_W:
    return F("Power (W)");
  case P084_QUERY_Wh_imp:
    return F("Import Energy (Wh)");
  case P084_QUERY_Wh_exp:
    return F("Export Energy (Wh)");
  case P084_QUERY_Wh_tot:
    return F("Total Energy (Wh)");
  case P084_QUERY_Wh_net:
    return F("Net Energy (Wh)");
  case P084_QUERY_h_tot:
    return F("Meter Running Time (h)");
  case P084_QUERY_h_load:
    return F("Load Running Time (h)");
  }
  return "";
}

String p084_getQueryValueString(byte query) {
  switch (query) {
  case P084_QUERY_V:
    return F("V");
  case P084_QUERY_A:
    return F("A");
  case P084_QUERY_W:
    return F("W");
  case P084_QUERY_Wh_imp:
    return F("Wh_imp");
  case P084_QUERY_Wh_exp:
    return F("Wh_exp");
  case P084_QUERY_Wh_tot:
    return F("Wh_tot");
  case P084_QUERY_Wh_net:
    return F("Wh_net");
  case P084_QUERY_h_tot:
    return F("h_tot");
  case P084_QUERY_h_load:
    return F("h_load");
  }
  return "";
}

int p084_storageValueToBaudrate(byte baudrate_setting) {
  switch (baudrate_setting) {
  case 0:
    return 1200;
  case 1:
    return 2400;
  case 2:
    return 4800;
  case 3:
    return 9600;
  case 4:
    return 19200;
  case 5:
    return 38400;
  }
  return 19200;
}

float p084_readValue(byte query, struct EventStruct *event) {
  P084_data_struct *P084_data =
      static_cast<P084_data_struct *>(getPluginTaskData(event->TaskIndex));
  if (nullptr != P084_data && P084_data->isInitialized()) {
    switch (query) {
    case P084_QUERY_V:
      return P084_data->modbus.read_float_HoldingRegister(0x200);
    case P084_QUERY_A:
      return P084_data->modbus.read_float_HoldingRegister(0x202);
    case P084_QUERY_W:
      return P084_data->modbus.read_float_HoldingRegister(0x204) * 1000.0; // power (kW => W)
    case P084_QUERY_Wh_imp:
      return P084_data->modbus.read_32b_HoldingRegister(0x300) * 10.0; // 0.01 kWh => Wh
    case P084_QUERY_Wh_exp:
      return P084_data->modbus.read_32b_HoldingRegister(0x302) * 10.0; // 0.01 kWh => Wh
    case P084_QUERY_Wh_tot:
      return P084_data->modbus.read_32b_HoldingRegister(0x304) * 10.0; // 0.01 kWh => Wh
    case P084_QUERY_Wh_net:
    {
      int64_t intvalue = P084_data->modbus.read_32b_HoldingRegister(0x306);
      if (intvalue >= (1<<31)) {
        intvalue = 4294967296 - intvalue;
      }
      float value = static_cast<float>(intvalue);
      value *= 10.0; // 0.01 kWh => Wh
      return value;
    }
    case P084_QUERY_h_tot:
      return P084_data->modbus.read_32b_HoldingRegister(0x280) / 100.0;
    case P084_QUERY_h_load:
      return P084_data->modbus.read_32b_HoldingRegister(0x282) / 100.0;
    }
  }
  return 0.0;
}

void p084_showValueLoadPage(byte query, struct EventStruct *event) {
  addRowLabel(p084_getQueryString(query));
  addHtml(String(p084_readValue(query, event)));
}


#endif // USES_P084
