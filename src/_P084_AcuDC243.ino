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
    }

    sensorTypeHelper_webformLoad_simple(event, P084_SENSOR_TYPE_INDEX);

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    serialHelper_webformSave(event);
    for (int i = 0; i < 7; ++i) {
      pconfig_webformSave(event, i);
    }
    sensorTypeHelper_saveSensorType(event, P084_SENSOR_TYPE_INDEX);

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
      sensorTypeHelper_setSensorType(event, P084_SENSOR_TYPE_INDEX);

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
      event->sensorType = PCONFIG(P084_SENSOR_TYPE_INDEX);

      uint32_t ival = 0;
      ival = P084_data->modbus.read_32b_HoldingRegister(0x200);
      float voltage = *reinterpret_cast<float*>(&ival);
      ival = P084_data->modbus.read_32b_HoldingRegister(0x202);
      float current = *reinterpret_cast<float*>(&ival);
      ival = P084_data->modbus.read_32b_HoldingRegister(0x204);
      float power = *reinterpret_cast<float*>(&ival);
      ival = P084_data->modbus.read_32b_HoldingRegister(0x300);
      float import_energy = *reinterpret_cast<float*>(&ival);

      UserVar[event->BaseVarIndex] = voltage;
      UserVar[event->BaseVarIndex + 1] = current;
      UserVar[event->BaseVarIndex + 2] = power;
      UserVar[event->BaseVarIndex + 3] = import_energy;

      success = true;
    }
    break;
  }
  }
  return success;
}

String p084_getQueryString(byte query) {
  switch (query) {
  case 0:
    return F("Voltage (V)");
  case 1:
    return F("Current (A)");
  case 2:
    return F("Power (W)");
  case 3:
    return F("Import Active Energy (Wh)");
  case 4:
    return F("Export Active Energy (Wh)");
  case 5:
    return F("Total Active Energy (Wh)");
  case 6:
    return F("Meter Running Time (h)");
  case 7:
    return F("Load Running Time (h)");
  }
  return "";
}

String p084_getQueryValueString(byte query) {
  switch (query) {
  case 0:
    return F("V");
  case 1:
    return F("A");
  case 2:
    return F("W");
  case 3:
    return F("Wh_imp");
  case 4:
    return F("Wh_exp");
  case 5:
    return F("Wh_tot");
  case 6:
    return F("h_tot");
  case 7:
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

#endif // USES_P084
