#include "_Plugin_Helper.h"
#ifdef USES_P108

# include "src/PluginStructs/P108_data_struct.h"

// ####################################################################################################
// ############################# Plugin 108: DDS238-x ZN ##############################################
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


/*
   DF - Below doesn't look right; needs a RS485 to TTL(3.3v) level converter (see https://github.com/reaper7/SDM_Energy_Meter)
   Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    GPIO Setting 3 -> DE/RE pin for MAX485
    Use 1kOhm in serie on datapins!
 */

# define PLUGIN_108
# define PLUGIN_ID_108 108
# define PLUGIN_NAME_108 "Energy (AC) - DDS238-x ZN"
# define PLUGIN_VALUENAME1_108 ""


boolean Plugin_108(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_108;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = P108_NR_OUTPUT_VALUES;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Simple;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_108);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P108_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P108_QUERY1_CONFIG_POS;
          uint8_t choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_108_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      event->String3 = formatGpioName_output_optional(F("DE"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS: {
      P108_DEV_ID   = P108_DEV_ID_DFLT;
      P108_MODEL    = P108_MODEL_DFLT;
      P108_BAUDRATE = P108_BAUDRATE_DFLT;
      P108_QUERY1   = P108_QUERY1_DFLT;
      P108_QUERY2   = P108_QUERY2_DFLT;
      P108_QUERY3   = P108_QUERY3_DFLT;
      P108_QUERY4   = P108_QUERY4_DFLT;

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
        options_baudrate[i] = String(p108_storageValueToBaudrate(i));
      }
      addFormSelector(F("Baud Rate"), P108_BAUDRATE_LABEL, 4, options_baudrate, nullptr, P108_BAUDRATE);
      addUnit(F("baud"));
      addFormNumericBox(F("Modbus Address"), P108_DEV_ID_LABEL, P108_DEV_ID, 1, 247);
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *options[P108_NR_OUTPUT_OPTIONS];

      for (int i = 0; i < P108_NR_OUTPUT_OPTIONS; ++i) {
        options[i] = Plugin_108_valuename(i, true);
      }

      for (uint8_t i = 0; i < P108_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P108_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P108_NR_OUTPUT_OPTIONS, options);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      P108_data_struct *P108_data =
        static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P108_data) && P108_data->isInitialized()) {
        addFormNote(P108_data->modbus.detected_device_description);
        addRowLabel(F("Checksum (pass/fail/nodata)"));
        uint32_t reads_pass, reads_crc_failed, reads_nodata;
        P108_data->modbus.getStatistics(reads_pass, reads_crc_failed, reads_nodata);
        String chksumStats;
        chksumStats  = reads_pass;
        chksumStats += '/';
        chksumStats += reads_crc_failed;
        chksumStats += '/';
        chksumStats += reads_nodata;
        addHtml(chksumStats);

        addFormSubHeader(F("Logged Values"));
        p108_showValueLoadPage(P108_QUERY_Wh_imp, event);
        p108_showValueLoadPage(P108_QUERY_Wh_exp, event);
        p108_showValueLoadPage(P108_QUERY_Wh_tot, event);
        p108_showValueLoadPage(P108_QUERY_V,      event);
        p108_showValueLoadPage(P108_QUERY_A,      event);
        p108_showValueLoadPage(P108_QUERY_W,      event);
        p108_showValueLoadPage(P108_QUERY_VA,     event);
        p108_showValueLoadPage(P108_QUERY_PF,     event);
        p108_showValueLoadPage(P108_QUERY_F,      event);

        // Can't clear totals, maybe because of modbus library can't write DWORD?
        // Disabled for now
        // Checkbox is always presented unchecked.
        // Must check and save to clear the stored accumulated values in the sensor.
        // addFormCheckBox(F("Clear logged values"), F("p108_clear_log"), false);
        // addFormNote(F("Will clear all logged values when checked and saved"));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      //      serialHelper_webformSave(event); // DF - not present in P085

      // Save normal parameters
      for (int i = 0; i < P108_QUERY1_CONFIG_POS; ++i) {
        pconfig_webformSave(event, i);
      }

      // Save output selector parameters.
      for (uint8_t i = 0; i < P108_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P108_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_108_valuename(choice, false));
      }

      // Can't clear totals, maybe because of modbus library can't write DWORD?
      // Disabled for now

      /*P108_data_struct *P108_data =
         static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));
         if ((nullptr != P108_data) && P108_data->isInitialized()) {

         if (isFormItemChecked(F("p108_clear_log")))
         {
          // Clear all logged values in the meter.
          P108_data->modbus.writeMultipleRegisters(0x0, 0x00); // Clear Total Energy
          P108_data->modbus.writeMultipleRegisters(0x1, 0x00); // Clear Total Energy
          P108_data->modbus.writeMultipleRegisters(0x8, 0x00); // Clear Import Energy
          P108_data->modbus.writeMultipleRegisters(0x9, 0x00); // Clear Import Energy
          P108_data->modbus.writeMultipleRegisters(0xA, 0x00); // Clear Export Energy
          P108_data->modbus.writeMultipleRegisters(0xB, 0x00); // Clear Export Energy
         }
         }*/


      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P108_data_struct());
      P108_data_struct *P108_data =
        static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P108_data) {
        return success;
      }

      if (P108_data->init(port, serial_rx, serial_tx, P108_DEPIN,
                          p108_storageValueToBaudrate(P108_BAUDRATE),
                          P108_DEV_ID)) {
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
      P108_data_struct *P108_data =
        static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P108_data) && P108_data->isInitialized()) {
        for (int i = 0; i < P108_NR_OUTPUT_VALUES; ++i) {
          UserVar[event->BaseVarIndex + i] = p108_readValue(PCONFIG(i + P108_QUERY1_CONFIG_POS), event);
          delay(1);
        }

        success = true;
      }
      break;
    }

# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // FIXME TD-er: Same code as in P102, share in LoRa code.
      P108_data_struct *P108_data =
        static_cast<P108_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P108_data) && P108_data->isInitialized()) {
        // Matching JS code:
        // return decode(bytes, [header, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4],
        //   ['header', 'unit1', 'val_1', 'unit2', 'val_2', 'unit3', 'val_3', 'unit4', 'val_4']);
        for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
          const uint8_t pconfigIndex = i + P108_QUERY1_CONFIG_POS;
          const uint8_t choice       = PCONFIG(pconfigIndex);
          string += LoRa_addInt(choice, PackedData_uint8);
          string += LoRa_addFloat(UserVar[event->BaseVarIndex + i], PackedData_int32_1e4);
        }
        event->Par1 = 8; // valuecount

        success = true;
      }
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA
  }
  return success;
}

#endif // USES_P108
