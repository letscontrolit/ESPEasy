#include "_Plugin_Helper.h"
#ifdef USES_P103

// ####################################################################################################
// ############################# Plugin 103: DDS238-x ZN ##############################################
// ####################################################################################################
//
// Pluging for Energy Meters DDS238-x ZN with MODBUS Interface (RS485), sold in Aliexpress and other
//   similar sites usually under the brand "Hiking"
// Tested with DDS238-1 ZN model, but should work with "-2" and "-4" versions as they have the same
//   register map.
// 
//
//  Written by José Araújo (josemariaaraujo@gmail.com),
//      with most code copied from plugin 085: _P085_AcuDC243.ino

#ifndef USES_MODBUS
#error This code needs MODBUS library, it should be enabled in 'define_plugin_sets.h', or your 'custom.h'
#endif

/*
DF - Below doesn't look right; needs a RS485 to TTL(3.3v) level converter (see https://github.com/reaper7/SDM_Energy_Meter)
   Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    GPIO Setting 3 -> DE/RE pin for MAX485
    Use 1kOhm in serie on datapins!
 */

#define PLUGIN_103
#define PLUGIN_ID_103 103
#define PLUGIN_NAME_103 "Energy Meter - DDS238-x ZN [TESTING]"
#define PLUGIN_VALUENAME1_103 ""

#define P103_DEV_ID         PCONFIG(0)
#define P103_DEV_ID_LABEL   PCONFIG_LABEL(0)
#define P103_MODEL          PCONFIG(1)
#define P103_MODEL_LABEL    PCONFIG_LABEL(1)
#define P103_BAUDRATE       PCONFIG(2)
#define P103_BAUDRATE_LABEL PCONFIG_LABEL(2)
#define P103_QUERY1         PCONFIG(3)
#define P103_QUERY2         PCONFIG(4)
#define P103_QUERY3         PCONFIG(5)
#define P103_QUERY4         PCONFIG(6)
#define P103_DEPIN          CONFIG_PIN3

#define P103_NR_OUTPUT_VALUES   VARS_PER_TASK
#define P103_QUERY1_CONFIG_POS  3

#define P103_QUERY_V       0
#define P103_QUERY_A       1
#define P103_QUERY_W       2
#define P103_QUERY_VA      3
#define P103_QUERY_PF      4
#define P103_QUERY_F       5
#define P103_QUERY_Wh_imp  6
#define P103_QUERY_Wh_exp  7
#define P103_QUERY_Wh_tot  8
#define P103_NR_OUTPUT_OPTIONS  9 // Must be the last one

#define P103_DEV_ID_DFLT   1       // Modbus communication address
#define P103_MODEL_DFLT    0       // DDS238
#define P103_BAUDRATE_DFLT 3       // 9600 baud
#define P103_QUERY1_DFLT   P103_QUERY_V
#define P103_QUERY2_DFLT   P103_QUERY_A
#define P103_QUERY3_DFLT   P103_QUERY_W
#define P103_QUERY4_DFLT   P103_QUERY_Wh_tot

#define P103_MEASUREMENT_INTERVAL 60000L

#include <ESPeasySerial.h>
#include "src/Helpers/Modbus_RTU.h"

struct P103_data_struct : public PluginTaskData_base {
  P103_data_struct() {}

  ~P103_data_struct() {
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

unsigned int _plugin_103_last_measurement = 0;

boolean Plugin_103(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_103;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = P103_NR_OUTPUT_VALUES;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_103);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P103_NR_OUTPUT_VALUES) {
          const byte pconfigIndex = i + P103_QUERY1_CONFIG_POS;
          byte choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_103_valuename(choice, false),
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
      P103_DEV_ID   = P103_DEV_ID_DFLT;
      P103_MODEL    = P103_MODEL_DFLT;
      P103_BAUDRATE = P103_BAUDRATE_DFLT;
      P103_QUERY1   = P103_QUERY1_DFLT;
      P103_QUERY2   = P103_QUERY2_DFLT;
      P103_QUERY3   = P103_QUERY3_DFLT;
      P103_QUERY4   = P103_QUERY4_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
        String options_baudrate[4];

        for (int i = 0; i < 4; ++i) {
          options_baudrate[i] = String(p103_storageValueToBaudrate(i));
        }
        addFormSelector(F("Baud Rate"), P103_BAUDRATE_LABEL, 4, options_baudrate, NULL, P103_BAUDRATE);
        addUnit(F("baud"));
        addFormNumericBox(F("Modbus Address"), P103_DEV_ID_LABEL, P103_DEV_ID, 1, 247);
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      P103_data_struct *P103_data =
        static_cast<P103_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P103_data) && P103_data->isInitialized()) {
        String detectedString = P103_data->modbus.detected_device_description;

        if (detectedString.length() > 0) {
          addFormNote(detectedString);
        }
        addRowLabel(F("Checksum (pass/fail/nodata)"));
        uint32_t reads_pass, reads_crc_failed, reads_nodata;
        P103_data->modbus.getStatistics(reads_pass, reads_crc_failed, reads_nodata);
        String chksumStats;
        chksumStats  = reads_pass;
        chksumStats += '/';
        chksumStats += reads_crc_failed;
        chksumStats += '/';
        chksumStats += reads_nodata;
        addHtml(chksumStats);

        addFormSubHeader(F("Logged Values"));
        p103_showValueLoadPage(P103_QUERY_Wh_imp, event);
        p103_showValueLoadPage(P103_QUERY_Wh_exp, event);
        p103_showValueLoadPage(P103_QUERY_Wh_tot, event);
        p103_showValueLoadPage(P103_QUERY_V, event);
        p103_showValueLoadPage(P103_QUERY_A, event);
        p103_showValueLoadPage(P103_QUERY_W, event);
        p103_showValueLoadPage(P103_QUERY_VA, event);
        p103_showValueLoadPage(P103_QUERY_PF, event);
        p103_showValueLoadPage(P103_QUERY_F, event);

        // Can't clear totals, maybe because of modbus library can't write DWORD?
        // Disabled for now
        // Checkbox is always presented unchecked.
        // Must check and save to clear the stored accumulated values in the sensor.
        //addFormCheckBox(F("Clear logged values"), F("p103_clear_log"), false);
        //addFormNote(F("Will clear all logged values when checked and saved"));
      }

      {
        // In a separate scope to free memory of String array as soon as possible
        sensorTypeHelper_webformLoad_header();
        String options[P103_NR_OUTPUT_OPTIONS];

        for (int i = 0; i < P103_NR_OUTPUT_OPTIONS; ++i) {
          options[i] = Plugin_103_valuename(i, true);
        }

        for (byte i = 0; i < P103_NR_OUTPUT_VALUES; ++i) {
          const byte pconfigIndex = i + P103_QUERY1_CONFIG_POS;
          sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P103_NR_OUTPUT_OPTIONS, options);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
//      serialHelper_webformSave(event); // DF - not present in P085

      // Save normal parameters
      for (int i = 0; i < P103_QUERY1_CONFIG_POS; ++i) {
        pconfig_webformSave(event, i);
      }

      // Save output selector parameters.
      for (byte i = 0; i < P103_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P103_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_103_valuename(choice, false));
      }
      // Can't clear totals, maybe because of modbus library can't write DWORD?
      // Disabled for now
      /*P103_data_struct *P103_data =
        static_cast<P103_data_struct *>(getPluginTaskData(event->TaskIndex));
      if ((nullptr != P103_data) && P103_data->isInitialized()) {

        if (isFormItemChecked(F("p103_clear_log")))
        {
          // Clear all logged values in the meter. 
          P103_data->modbus.writeMultipleRegisters(0x0, 0x00); // Clear Total Energy
          P103_data->modbus.writeMultipleRegisters(0x1, 0x00); // Clear Total Energy
          P103_data->modbus.writeMultipleRegisters(0x8, 0x00); // Clear Import Energy      
          P103_data->modbus.writeMultipleRegisters(0x9, 0x00); // Clear Import Energy
          P103_data->modbus.writeMultipleRegisters(0xA, 0x00); // Clear Export Energy
          P103_data->modbus.writeMultipleRegisters(0xB, 0x00); // Clear Export Energy            
        }
      }*/


      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P103_data_struct());
      P103_data_struct *P103_data =
        static_cast<P103_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P103_data) {
        return success;
      }

      if (P103_data->init(port, serial_rx, serial_tx, P103_DEPIN,
                          p103_storageValueToBaudrate(P103_BAUDRATE),
                          P103_DEV_ID)) {
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
        success = true;
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_EXIT: {
//       clearPluginTaskData(event->TaskIndex); // DF - not present in P085
      success = true;
      break;
    }

    case PLUGIN_READ: {
      P103_data_struct *P103_data =
        static_cast<P103_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P103_data) && P103_data->isInitialized()) {
        for (int i = 0; i < P103_NR_OUTPUT_VALUES; ++i) {
          UserVar[event->BaseVarIndex + i] = p103_readValue(PCONFIG(i + P103_QUERY1_CONFIG_POS), event);
          delay(1);
        }

        success = true;
      }
      break;
    }
  }
  return success;
}

String Plugin_103_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case P103_QUERY_V: return displayString ? F("Voltage (V)") : F("V");
    case P103_QUERY_A: return displayString ? F("Current (A)") : F("A");
    case P103_QUERY_W: return displayString ? F("Active Power (W)") : F("W");
    case P103_QUERY_VA: return displayString ? F("Reactive Power (VA)") : F("VA");    
    case P103_QUERY_PF: return displayString ? F("Power Factor (Pf)") : F("Pf");
    case P103_QUERY_F: return displayString ? F("Frequency (Hz)") : F("Hz");
    case P103_QUERY_Wh_imp: return displayString ? F("Import Energy (Wh)") : F("Wh_imp");
    case P103_QUERY_Wh_exp: return displayString ? F("Export Energy (Wh)") : F("Wh_exp");
    case P103_QUERY_Wh_tot: return displayString ? F("Total Energy (Wh)") : F("Wh_tot");
  }
  return "";
}

int p103_storageValueToBaudrate(byte baudrate_setting) {
  switch (baudrate_setting) {
    case 0:
      return 1200;
    case 1:
      return 2400;
    case 2:
      return 4800;
    case 3:
      return 9600;
  }
  return 9600;
}

float p103_readValue(byte query, struct EventStruct *event) {
  byte errorcode     = -1; // DF - not present in P085
  float value = 0; // DF - not present in P085
  P103_data_struct *P103_data =
    static_cast<P103_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P103_data) && P103_data->isInitialized()) {
    switch (query) {
      case P103_QUERY_V:
        value = P103_data->modbus.readHoldingRegister(0x0C ,errorcode) / 10.0; // 0.1 V => V
        break;
      case P103_QUERY_A:
        value = P103_data->modbus.readHoldingRegister(0x0D, errorcode) / 100.0; // 0.01 A => A
        break;
      case P103_QUERY_W:
        value =  P103_data->modbus.readHoldingRegister(0x0E, errorcode) * 1.0 ;
        break;
      case P103_QUERY_VA:
        value = P103_data->modbus.readHoldingRegister(0x0F, errorcode) * 1.0 ;
        break;
      case P103_QUERY_PF:
        value = P103_data->modbus.readHoldingRegister(0x10, errorcode) / 1000.0; // 0.001 Pf => Pf
        break;
      case P103_QUERY_F:
        value = P103_data->modbus.readHoldingRegister(0x11, errorcode) / 100.0 ; // 0.01 Hz => Hz
        break;
      case P103_QUERY_Wh_imp:
        return P103_data->modbus.read_32b_HoldingRegister(0x0A) * 10.0;     // 0.01 kWh => Wh
        break;
      case P103_QUERY_Wh_exp:
        return P103_data->modbus.read_32b_HoldingRegister(0x08) * 10.0;     // 0.01 kWh => Wh
        break;
      case P103_QUERY_Wh_tot:
        return P103_data->modbus.read_32b_HoldingRegister(0x00) * 10.0;     // 0.01 kWh => Wh
        break;
    }
  }
  if (errorcode == 0) return value; // DF - not present in P085
  return 0.0f;
}

void p103_showValueLoadPage(byte query, struct EventStruct *event) {
  addRowLabel(Plugin_103_valuename(query, true));
  addHtml(String(p103_readValue(query, event)));
}

#endif // USES_P103
