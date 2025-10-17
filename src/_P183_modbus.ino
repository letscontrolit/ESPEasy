#include "_Plugin_Helper.h"
#ifdef USES_P183

// #######################################################################################################
// ############## Plugin 183: Modbus RTU generic sensor interface                          ###############
// #######################################################################################################

/*
   Plugin written by: Flashmark

   This plugin reads values from a generic Modbus RTU device. It sees the device as a series of registers.
   Up to 4 registers can be monitored and presented as standard output values of the plugin.
   The plugin also provides means to write register using the PLUGIN_WRITE commands.
   For debugging the Modbus and accessing other registers additional commands are available.
   This plugin uses a generic MODBUS_FAC facility to share a single Modbus link with multiple device instances.
 */

/**
 * Changelog:
 * 2025-08-24 flashmark: Initial version
 * 2025-10-12 flashmark: Restructuring and adding a MODBUS_FAC facility
 */

# define P183_DEBUG // Switch on additional debug logging
# ifdef BUILD_NO_DEBUG
#  undef P183_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG
# define PLUGIN_183
# define PLUGIN_ID_183         183
# define PLUGIN_NAME_183       "[testing] Modbus RTU"
# define P183_NR_OUTPUT_VALUES 4
# define PLUGIN_VALUENAME1_183 "Value1"
# define PLUGIN_VALUENAME2_183 "Value2"
# define PLUGIN_VALUENAME3_183 "Value3"
# define PLUGIN_VALUENAME4_183 "Value4"

# include <ESPeasySerial.h>
# include "src/PluginStructs/P183_data_struct.h"
# include "src/Helpers/Modbus_device.h"
# include "src/Helpers/Modbus_mgr.h"

boolean Plugin_183(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_183;
      dev.Type             = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      dev.VType            = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption    = true;
      dev.ValueCount       = P183_NR_OUTPUT_VALUES;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_183);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_183));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_183));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_modbus_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P183_DEV_ID   = P183_DEV_ID_DFLT;
      P183_BAUDRATE = P183_BAUDRATE_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((P183_DEV_ID <= 0) || (P183_DEV_ID > P183_MAX_MODBUS_NODES) || (P183_BAUDRATE >= 6)) {
        // Load some defaults
        P183_DEV_ID   = P183_DEV_ID_DFLT;
        P183_BAUDRATE = P183_BAUDRATE_DFLT;
      }
      {
        String options_baudrate[P183_MAX_BAUDRATE_SEL];

        for (int i = 0; i < P183_MAX_BAUDRATE_SEL; ++i) {
          options_baudrate[i] = P183_storageValueToBaudrate(i);
        }
        constexpr size_t optionCount = NR_ELEMENTS(options_baudrate);
        const FormSelectorOptions selector(optionCount, options_baudrate);
        selector.addFormSelector(F("Baud Rate"), P183_BAUDRATE_LABEL, P183_BAUDRATE);
        addUnit(F("baud"));
      }

      addFormNumericBox(F("Modbus Device Address"), P183_DEV_ID_LABEL, P183_DEV_ID, 1, 247);

      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), F(P183_FLAG_COLL_DETECT_LABEL), P183_GET_FLAG_COLL_DETECT);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      if ((P183_NR_OUTPUTS < 1) || (P183_NR_OUTPUTS > P183_NR_OUTPUT_VALUES)) {
        P183_NR_OUTPUTS = P183_NR_OUTPUT_VALUES; // Default to max outputs
      }
      addFormNumericBox(F("Number of values to read"), P183_NR_OUTPUTS_LABEL, P183_NR_OUTPUTS);

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUT_VALUES; ++outputIndex)
      {
        addFormNumericBox(concat(F("Holding Register for value"), outputIndex + 1), P183_ADDRESS_LABEL(outputIndex),
                          P183_ADDRESS(outputIndex));
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P183_DEV_ID   = getFormItemInt(P183_DEV_ID_LABEL);
      P183_BAUDRATE = getFormItemInt(P183_BAUDRATE_LABEL);
      # ifdef ESP32
      P183_SET_FLAG_COLL_DETECT(isFormItemChecked(F(P183_FLAG_COLL_DETECT_LABEL)));
      # endif // ifdef ESP32

      P183_NR_OUTPUTS = getFormItemInt(P183_NR_OUTPUTS_LABEL);

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUT_VALUES; ++outputIndex)
      {
        P183_ADDRESS(outputIndex) = getFormItemInt(P183_ADDRESS_LABEL(outputIndex));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P183_data_struct(event));
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data  != nullptr) {
        success = P183_data->plugin_init(P183_DEV_ID,
                                         static_cast<ESPEasySerialPort>(CONFIG_PORT),
                                         CONFIG_PIN1,
                                         CONFIG_PIN2,
                                         P183_storageValueToBaudrate(P183_BAUDRATE),
                                         P183_DEPIN,
                                         P183_GET_FLAG_COLL_DETECT);
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("P183 : Cannot initialize"));
      }

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P183_data) {
        P183_data->plugin_exit();
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        addLogMove(LOG_LEVEL_INFO, F("******* Modbus: Read invalid data struct"));
        return false;
      }
      success = P183_data->plugin_read(event); // Delegate to data_struct
      break;
    }
    case PLUGIN_WRITE:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        addLogMove(LOG_LEVEL_INFO, F("******* Modbus: Write invalid data struct"));
        return false;
      }

      const String cmd = parseString(string, 1);

      if (equals(cmd, F("modbus"))) {
        const String subcmd = parseString(string, 2);

        if (equals(subcmd, F("write"))) {
          // Write a value to a Modbus register
          int address    = event->Par2;
          uint16_t value = event->Par3;
          P183_data->writeRegister(address, value);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("Modbus: write value %u to address 0x%04x"), value, address));
          }
          success = true;
        }
        else if (equals(subcmd, F("read"))) {
          // Read a value from a Modbus register
          int address    = event->Par2;
          uint16_t value = 0;
          value = P183_data->readRegisterWait(address);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("Modbus: read value %u from address 0x%04x"), value, address));
          }
          success = true;
        }
        else if (equals(subcmd, F("dump"))) {
          int start_address = event->Par2;
          int end_address   = event->Par3;

          if (end_address < start_address) {
            end_address = start_address;
          }

          if (end_address - start_address > 100) {
            end_address = start_address + 100; // Limit to 100 addresses
          }
          P183_data->scan_device(P183_DEV_ID, start_address, end_address);
          success = true;
        }
        else if (equals(subcmd, F("scan"))) {
          // Scan for Modbus devices
          P183_data->scan_modbus();
          success = true;
        }
        else {
          addLogMove(LOG_LEVEL_ERROR, F("Modbus: Unknown command"));
        }
      }

      break;
    }
    case PLUGIN_GET_CONFIG_VALUE: {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        addLogMove(LOG_LEVEL_INFO, F("******* Modbus: Get config invalid data struct"));
        return false;
      }

      const String cmd = parseString(string, 1);

      if (equals(cmd, F("register"))) {
        int address    = parseString(string, 2).toInt();
        uint16_t value = 0;
        value   = P183_data->readRegisterWait(address);
        string  = String(value);
        success = true;
      }
      break;
    }
    case PLUGIN_TEN_PER_SECOND: {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data != nullptr) {
        P183_data->plugin_ten_per_second(event);
      }
      break;
    }
  }

  return success;
}

// Convert stored baudrate setting (enumeration value) to actual baudrate value
// Returns the actual baudrate value.
int P183_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  switch (baudrate_setting)
  {
    case 0:
      baudrate = 1200;   break;
    case 1:
      baudrate = 2400;   break;
    case 2:
      baudrate = 4800;   break;
    case 3:
      baudrate = 9600;   break;
    case 4:
      baudrate = 19200;  break;
    case 5:
      baudrate = 38400;  break;
    case 6:
      baudrate = 57600;  break;
    case 7:
      baudrate = 115200; break;
    default:
      baudrate = 9600;   break; // Default value for fallback
  }
  return baudrate;
}

#endif // USES_P183
