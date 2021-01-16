#include "_Plugin_Helper.h"
#ifdef USES_P085

// #######################################################################################################
// ############################# Plugin 085: AccuEnergy AcuDC24x #########################################
// #######################################################################################################


/*

   Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    GPIO Setting 3 -> DE/RE pin for MAX485
    Use 1kOhm in serie on datapins!
 */

#define PLUGIN_085
#define PLUGIN_ID_085 85
#define PLUGIN_NAME_085 "Energy - AccuEnergy AcuDC24x [TESTING]"
#define PLUGIN_VALUENAME1_085 ""

#define P085_DEV_ID         PCONFIG(0)
#define P085_DEV_ID_LABEL   PCONFIG_LABEL(0)
#define P085_MODEL          PCONFIG(1)
#define P085_MODEL_LABEL    PCONFIG_LABEL(1)
#define P085_BAUDRATE       PCONFIG(2)
#define P085_BAUDRATE_LABEL PCONFIG_LABEL(2)
#define P085_QUERY1         PCONFIG(3)
#define P085_QUERY2         PCONFIG(4)
#define P085_QUERY3         PCONFIG(5)
#define P085_QUERY4         PCONFIG(6)
#define P085_DEPIN          CONFIG_PIN3

#define P085_NR_OUTPUT_VALUES   VARS_PER_TASK
#define P085_QUERY1_CONFIG_POS  3

#define P085_QUERY_V       0
#define P085_QUERY_A       1
#define P085_QUERY_W       2
#define P085_QUERY_Wh_imp  3
#define P085_QUERY_Wh_exp  4
#define P085_QUERY_Wh_tot  5
#define P085_QUERY_Wh_net  6
#define P085_QUERY_h_tot   7
#define P085_QUERY_h_load  8
#define P085_NR_OUTPUT_OPTIONS   9 // Must be the last one

#define P085_DEV_ID_DFLT   1       // Modbus communication address
#define P085_MODEL_DFLT    0       // AcuDC24x
#define P085_BAUDRATE_DFLT 4       // 19200 baud
#define P085_QUERY1_DFLT   P085_QUERY_V
#define P085_QUERY2_DFLT   P085_QUERY_A
#define P085_QUERY3_DFLT   P085_QUERY_W
#define P085_QUERY4_DFLT   P085_QUERY_Wh_tot

#define P085_MEASUREMENT_INTERVAL 60000L

#include <ESPeasySerial.h>
#include "src/Helpers/Modbus_RTU.h"

struct P085_data_struct : public PluginTaskData_base {
  P085_data_struct() {}

  ~P085_data_struct() {
    reset();
  }

  void reset() {
    modbus.reset();
  }

  bool init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, int8_t dere_pin,
            unsigned int baudrate, uint8_t modbusAddress) {
    return modbus.init(port, serial_rx, serial_tx, baudrate, modbusAddress, dere_pin);
  }

  bool isInitialized() const {
    return modbus.isInitialized();
  }

  ModbusRTU_struct modbus;
};

unsigned int _plugin_085_last_measurement = 0;

boolean Plugin_085(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_085;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = P085_NR_OUTPUT_VALUES;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_085);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P085_NR_OUTPUT_VALUES) {
          const byte pconfigIndex = i + P085_QUERY1_CONFIG_POS;
          byte choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_085_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      event->String3 = formatGpioName_output_optional("DE");
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS: {
      P085_DEV_ID   = P085_DEV_ID_DFLT;
      P085_MODEL    = P085_MODEL_DFLT;
      P085_BAUDRATE = P085_BAUDRATE_DFLT;
      P085_QUERY1   = P085_QUERY1_DFLT;
      P085_QUERY2   = P085_QUERY2_DFLT;
      P085_QUERY3   = P085_QUERY3_DFLT;
      P085_QUERY4   = P085_QUERY4_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      String options_baudrate[6];

      for (int i = 0; i < 6; ++i) {
        options_baudrate[i] = String(p085_storageValueToBaudrate(i));
      }
      addFormSelector(F("Baud Rate"), P085_BAUDRATE_LABEL, 6, options_baudrate, NULL, P085_BAUDRATE);
      addUnit(F("baud"));
      addFormNumericBox(F("Modbus Address"), P085_DEV_ID_LABEL, P085_DEV_ID, 1, 247);
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      P085_data_struct *P085_data =
        static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P085_data) && P085_data->isInitialized()) {
        String detectedString = P085_data->modbus.detected_device_description;

        if (detectedString.length() > 0) {
          addFormNote(detectedString);
        }
        addRowLabel(F("Checksum (pass/fail/nodata)"));
        uint32_t reads_pass, reads_crc_failed, reads_nodata;
        P085_data->modbus.getStatistics(reads_pass, reads_crc_failed, reads_nodata);
        String chksumStats;
        chksumStats  = reads_pass;
        chksumStats += '/';
        chksumStats += reads_crc_failed;
        chksumStats += '/';
        chksumStats += reads_nodata;
        addHtml(chksumStats);

        addFormSubHeader(F("Calibration"));

        // Calibration data is stored in the AcuDC module, not in the settings of ESPeasy.
        {
          byte errorcode = 0;
          int  value     = P085_data->modbus.readHoldingRegister(0x107, errorcode);

          if (errorcode == 0) {
            addFormNumericBox(F("Full Range Voltage Value"), F("p085_fr_volt"), value, 5, 9999);
            addUnit(F("V"));
          }
          value = P085_data->modbus.readHoldingRegister(0x104, errorcode);

          if (errorcode == 0) {
            addFormNumericBox(F("Full Range Current Value"), F("p085_fr_curr"), value, 20, 50000);
            addUnit(F("A"));
          }
          value = P085_data->modbus.readHoldingRegister(0x105, errorcode);

          if (errorcode == 0) {
            addFormNumericBox(F("Full Range Shunt Value"), F("p085_fr_shunt"), value, 50, 100);
            addUnit(F("mV"));
          }

          addFormSubHeader(F("Logging"));

          value = P085_data->modbus.readHoldingRegister(0x500, errorcode);

          if (errorcode == 0) {
            addFormCheckBox(F("Enable data logging"), F("p085_en_log"), value);
          }
          value = P085_data->modbus.readHoldingRegister(0x501, errorcode);

          if (errorcode == 0) {
            addRowLabel(F("Mode of data logging"));
            addHtmlInt(value);
          }
          value = P085_data->modbus.readHoldingRegister(0x502, errorcode);

          if (errorcode == 0) {
            addFormNumericBox(F("Log Interval"), F("p085_log_int"), value, 1, 1440);
            addUnit(F("minutes"));
          }
        }

        addFormSubHeader(F("Logged Values"));
        p085_showValueLoadPage(P085_QUERY_Wh_imp, event);
        p085_showValueLoadPage(P085_QUERY_Wh_exp, event);
        p085_showValueLoadPage(P085_QUERY_Wh_tot, event);
        p085_showValueLoadPage(P085_QUERY_Wh_net, event);
        p085_showValueLoadPage(P085_QUERY_h_tot,  event);
        p085_showValueLoadPage(P085_QUERY_h_load, event);

        // Checkbox is always presented unchecked.
        // Must check and save to clear the stored accumulated values in the sensor.
        addFormCheckBox(F("Clear logged values"), F("p085_clear_log"), false);
        addFormNote(F("Will clear all logged values when checked and saved"));
      }

      {
        // In a separate scope to free memory of String array as soon as possible
        sensorTypeHelper_webformLoad_header();
        String options[P085_NR_OUTPUT_OPTIONS];

        for (int i = 0; i < P085_NR_OUTPUT_OPTIONS; ++i) {
          options[i] = Plugin_085_valuename(i, true);
        }

        for (byte i = 0; i < P085_NR_OUTPUT_VALUES; ++i) {
          const byte pconfigIndex = i + P085_QUERY1_CONFIG_POS;
          sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P085_NR_OUTPUT_OPTIONS, options);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {

      // Save normal parameters
      for (int i = 0; i < P085_QUERY1_CONFIG_POS; ++i) {
        pconfig_webformSave(event, i);
      }

      // Save output selector parameters.
      for (byte i = 0; i < P085_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P085_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_085_valuename(choice, false));
      }
      P085_data_struct *P085_data =
        static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P085_data) && P085_data->isInitialized()) {
        uint16_t log_enabled = isFormItemChecked(F("p085_en_log")) ? 1 : 0;
        P085_data->modbus.writeMultipleRegisters(0x500, log_enabled);
        delay(1);

        uint16_t log_int = getFormItemInt(F("p085_log_int"));
        P085_data->modbus.writeMultipleRegisters(0x502, log_int);
        delay(1);

        uint16_t current = getFormItemInt(F("p085_fr_curr"));
        P085_data->modbus.writeMultipleRegisters(0x104, current);
        delay(1);

        uint16_t shunt = getFormItemInt(F("p085_fr_shunt"));
        P085_data->modbus.writeMultipleRegisters(0x105, shunt);
        delay(1);

        uint16_t voltage = getFormItemInt(F("p085_fr_volt"));
        P085_data->modbus.writeMultipleRegisters(0x107, voltage);

        if (isFormItemChecked(F("p085_clear_log")))
        {
          // Clear all logged values in the meter.
          P085_data->modbus.writeMultipleRegisters(0x122, 0x0A); // Clear Energy
          P085_data->modbus.writeMultipleRegisters(0x123, 0x0A); // Clear Meter Running Hour
          P085_data->modbus.writeMultipleRegisters(0x124, 0x0A); // Clear Meter Load Hour
          P085_data->modbus.writeMultipleRegisters(0x127, 0x0A); // Clear Ah
          P085_data->modbus.writeMultipleRegisters(0x128, 0x0A); // Clear Min/Max value
          P085_data->modbus.writeMultipleRegisters(0x129, 0x0A); // Clear Data Logging
        }
      }


      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P085_data_struct());
      P085_data_struct *P085_data =
        static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P085_data) {
        return success;
      }

      if (P085_data->init(port, serial_rx, serial_tx, P085_DEPIN,
                          p085_storageValueToBaudrate(P085_BAUDRATE),
                          P085_DEV_ID)) {
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
        success = true;
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_READ: {
      P085_data_struct *P085_data =
        static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P085_data) && P085_data->isInitialized()) {
        for (int i = 0; i < P085_NR_OUTPUT_VALUES; ++i) {
          UserVar[event->BaseVarIndex + i] = p085_readValue(PCONFIG(i + P085_QUERY1_CONFIG_POS), event);
          delay(1);
        }

        success = true;
      }
      break;
    }
#ifdef USES_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      P085_data_struct *P085_data =
        static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P085_data) && P085_data->isInitialized()) {
        // Matching JS code:
        // return decode(bytes, [header, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4],
        //   ['header', 'unit1', 'val_1', 'unit2', 'val_2', 'unit3', 'val_3', 'unit4', 'val_4']);
        for (byte i = 0; i < VARS_PER_TASK; ++i) {
          const byte pconfigIndex = i + P085_QUERY1_CONFIG_POS;
          const byte choice       = PCONFIG(pconfigIndex);
          string += LoRa_addInt(choice, PackedData_uint8);
          string += LoRa_addFloat(UserVar[event->BaseVarIndex + i], PackedData_int32_1e4);
        }
        event->Par1 = 8; // valuecount 
        
        success = true;
      }
      break;
    }
#endif // USES_PACKED_RAW_DATA


  }
  return success;
}

String Plugin_085_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case P085_QUERY_V:      return displayString ? F("Voltage (V)") : F("V");
    case P085_QUERY_A:      return displayString ? F("Current (A)") : F("A");
    case P085_QUERY_W:      return displayString ? F("Power (W)") : F("W");
    case P085_QUERY_Wh_imp: return displayString ? F("Import Energy (Wh)") : F("Wh_imp");
    case P085_QUERY_Wh_exp: return displayString ? F("Export Energy (Wh)") : F("Wh_exp");
    case P085_QUERY_Wh_tot: return displayString ? F("Total Energy (Wh)") : F("Wh_tot");
    case P085_QUERY_Wh_net: return displayString ? F("Net Energy (Wh)") : F("Wh_net");
    case P085_QUERY_h_tot:  return displayString ? F("Meter Running Time (h)") : F("h_tot");
    case P085_QUERY_h_load: return displayString ? F("Load Running Time (h)") : F("h_load");
  }
  return "";
}

int p085_storageValueToBaudrate(byte baudrate_setting) {
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
      return 38500;
  }
  return 19200;
}

float p085_readValue(byte query, struct EventStruct *event) {
  P085_data_struct *P085_data =
    static_cast<P085_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P085_data) && P085_data->isInitialized()) {
    switch (query) {
      case P085_QUERY_V:
        return P085_data->modbus.read_float_HoldingRegister(0x200);
      case P085_QUERY_A:
        return P085_data->modbus.read_float_HoldingRegister(0x202);
      case P085_QUERY_W:
        return P085_data->modbus.read_float_HoldingRegister(0x204) * 1000.0f; // power (kW => W)
      case P085_QUERY_Wh_imp:
        return P085_data->modbus.read_32b_HoldingRegister(0x300) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_exp:
        return P085_data->modbus.read_32b_HoldingRegister(0x302) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_tot:
        return P085_data->modbus.read_32b_HoldingRegister(0x304) * 10.0f;     // 0.01 kWh => Wh
      case P085_QUERY_Wh_net:
      {
        int64_t intvalue = P085_data->modbus.read_32b_HoldingRegister(0x306);

        if (intvalue >= 2147483648ll) {
          intvalue = 4294967296ll - intvalue;
        }
        float value = static_cast<float>(intvalue);
        value *= 10.0f; // 0.01 kWh => Wh
        return value;
      }
      case P085_QUERY_h_tot:
        return P085_data->modbus.read_32b_HoldingRegister(0x280) / 100.0f;
      case P085_QUERY_h_load:
        return P085_data->modbus.read_32b_HoldingRegister(0x282) / 100.0f;
    }
  }
  return 0.0f;
}

void p085_showValueLoadPage(byte query, struct EventStruct *event) {
  addRowLabel(Plugin_085_valuename(query, true));
  addHtml(String(p085_readValue(query, event)));
}

#endif // USES_P085
