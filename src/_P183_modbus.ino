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
 * 2026-04-29 flashmark: Refactor for new modbus facility using separated Modbus link object.
 * 2026-04-13 flashmark: Separate Modbus link definition from plugin.
 * 2025-10-12 flashmark: Restructuring and adding a MODBUS_FAC facility
 * 2025-08-24 flashmark: Initial version
 */

//// # define P183_DEBUG

# define PLUGIN_183
# define PLUGIN_ID_183         183
# define PLUGIN_NAME_183       "Communication - Modbus RTU"
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
      dev.Type             = DEVICE_TYPE_CUSTOM0; // Custom device type, connects to Modbus
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

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(P183_NR_OUTPUTS));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(P183_NR_OUTPUTS);
      event->idx        = P183_NR_OUTPUTS_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P183_DEV_ID     = P183_DEV_ID_DFLT;
      P183_LINK_ID    = P183_LINK_ID_DFLT;
      P183_NR_OUTPUTS = P183_NR_OUTPUT_VALUES; // Default to max outputs
      P183_CACHE_SIZE = 0;                     // Default to no cache
      success         = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      //     if ((P183_NR_OUTPUTS < 1) || (P183_NR_OUTPUTS > P183_NR_OUTPUT_VALUES)) {
      //       P183_NR_OUTPUTS = P183_NR_OUTPUT_VALUES; // Default to max outputs
      //     }
      //      addFormNumericBox(F("Number of values to read"), P183_NR_OUTPUTS_LABEL, P183_NR_OUTPUTS);

      for (int outputIndex = 0; outputIndex < P183_NR_OUTPUT_VALUES; ++outputIndex)
      {
        addFormNumericBox(concat(F("Holding Register for value"), outputIndex + 1), P183_ADDRESS_LABEL(outputIndex),
                          P183_ADDRESS(outputIndex));
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Modbus Link"),           P183_LINK_ID_LABEL,     P183_LINK_ID,     0, 3);
      addFormNumericBox(F("Modbus Device Address"), P183_DEV_ID_LABEL,      P183_DEV_ID,      1, P183_MAX_MODBUS_NODES);
      addFormNumericBox(F("Cache size"),            P183_CACHE_SIZE_LABEL,  P183_CACHE_SIZE,  0, P183_CACHE_SIZE_MAX);
      addFormNumericBox(F("Cache start address"),   P183_CACHE_START_LABEL, P183_CACHE_START, 0, P183_CACHE_START_MAX);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P183_DEV_ID      = getFormItemInt(P183_DEV_ID_LABEL);
      P183_LINK_ID     = getFormItemInt(P183_LINK_ID_LABEL);
      P183_NR_OUTPUTS  = getFormItemInt(P183_NR_OUTPUTS_LABEL);
      P183_CACHE_START = getFormItemInt(P183_CACHE_START_LABEL);
      P183_CACHE_SIZE  = getFormItemInt(P183_CACHE_SIZE_LABEL);

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
        success = P183_data->plugin_init(P183_DEV_ID, P183_LINK_ID);
      }
      else {
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, F("P183 : Cannot initialize"));
        # endif // LIMIT_BUILD_SIZE
        success = false;
      }
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
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, F("P183 : Modbus read invalid data struct"));
        # endif // LIMIT_BUILD_SIZE
        return false;
      }
      success = P183_data->plugin_read(event); // Delegate to data_struct
      break;
    }

    case PLUGIN_WRITE:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, F("P183 : Modbus write invalid data struct"));
        # endif // LIMIT_BUILD_SIZE
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
          # ifdef P183_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("P183 : Modbus write value %u to address 0x%04x"), value, address));
          }
          # endif // P183_DEBUG
          success = true;
        }
        else if (equals(subcmd, F("read"))) {
          // Read a value from a Modbus register
          uint16_t address = event->Par2;
          uint16_t value   = 0;

          if ((address >= P183_CACHE_START) && (address < P183_CACHE_START + P183_CACHE_SIZE)) {
            value = P183_data->readRegisterCache(address);
          }
          else {
            # if P183_ALLOW_MODBUS_WAIT
            value = P183_data->readRegisterWait(address); // Warning: this may take time as we waith for the  Modbus message to be exchanged
            # else
            addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus read command with invalid address"), event->TaskIndex + 1));
            value = 0;
            # endif // if P183_ALLOW_MODBUS_WAIT

          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO,
                       strformat(F("P183[%d]: Modbus read value %u from address 0x%04x"), event->TaskIndex + 1, value, address));
          }
          success = true;
        }
        else if (equals(subcmd, F("dump"))) {
          uint16_t start_address = event->Par2;
          uint16_t end_address   = event->Par3;

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
        else if (equals(subcmd, F("debug"))) {
          // Dump Modbus admin info
          ModbusMGR_singleton.dumpAdminInfo();
          success = true;
        }
        # ifndef LIMIT_BUILD_SIZE
        else {
          addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus Unknown command"), event->TaskIndex + 1));
        }
        # endif // LIMIT_BUILD_SIZE
      }

      break;
    }

    // Event from Modbus device for a response received on a queued request
    case PLUGIN_TASKTIMER_IN:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus task timer invalid data struct"), event->TaskIndex + 1));
        # endif // LIMIT_BUILD_SIZE
        return false;
      }
      success = P183_data->plugin_task_timer(event); // Delegate to data_struct
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus Get config invalid data struct"), event->TaskIndex + 1));
        # endif // LIMIT_BUILD_SIZE
        return false;
      }

      const String cmd = parseString(string, 1, '.');

      if (equals(cmd, F("register"))) {
        uint32_t address{};

        if (validUIntFromString(parseString(string, 2, '.'), address)) {
          uint16_t value = 0;

          if ((address >= P183_CACHE_START) && (address < P183_CACHE_START + P183_CACHE_SIZE)) {
            value = P183_data->readRegisterCache(address);
          }
          else {
          # if P183_ALLOW_MODBUS_WAIT
            value = P183_data->readRegisterWait(address); // Warning: this may take time as we waith for the  Modbus message to be exchanged
          # else
            addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus Get config command invalid address"), event->TaskIndex + 1));
            value = 0;
          # endif // if P183_ALLOW_MODBUS_WAIT
          }
          string  = String(value);
          success = true;
        }
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += strformat(F("Modbus %d<br>Addr: %d"), P183_LINK_ID, P183_DEV_ID);
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P183_data_struct *P183_data = static_cast<P183_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P183_data == nullptr) {
        # ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, strformat(F("P183[%d]: Modbus once per second invalid data struct"), event->TaskIndex + 1));
        # endif // LIMIT_BUILD_SIZE
        return false;
      }

      success = P183_data->plugin_once_per_second(event);
      break;
    }

  }

  return success;
}

#endif // USES_P183
