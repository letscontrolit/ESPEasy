/*
 * Plugin 115 MAX1704x I2C, Sparkfun Fuel Gauge Sensor
 *
 * development version
 * by: jbaream
 * this plugin is based in example1 by Paul Clark
 * from SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library
 * provides Battery Voltage, Battery State of Charge (SOC) and Alert Flag when SOC is below a threshold
 * defined in device configuration webform
 *
 * Datasheet: https://datasheets.maximintegrated.com/en/ds/MAX17043-MAX17044.pdf
 *
 */
#include "_Plugin_Helper.h"
#ifdef USES_P115


# include "src/PluginStructs/P115_data_struct.h"

# define PLUGIN_115
# define PLUGIN_ID_115     115           // plugin id
# define PLUGIN_NAME_115   "Energy - Fuel Gauge MAX1704x [TESTING]"
# define PLUGIN_VALUENAME1_115 "Voltage" // Battery voltage
# define PLUGIN_VALUENAME2_115 "SOC"     // Battery state of charge in percentage
# define PLUGIN_VALUENAME3_115 "Alert"   // (0 or 1) Alert when the battery SoC gets too low
# define PLUGIN_VALUENAME4_115 "Rate"    // (MAX17048/49) Get rate of change per hour in %
# define PLUGIN_xxx_DEBUG  false         // set to true for extra log info in the debug

boolean Plugin_115(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_115;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;                  // how the device is connected
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE; // type of value the plugin will return
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4; 
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;

      // Device[deviceCount].TimerOptional = false;
      Device[deviceCount].GlobalSyncOption = true;
      Device[deviceCount].DecimalsOnly     = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_115);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_115));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_115));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_115));
      break;
    }

    case PLUGIN_INIT:
    {
      const sfe_max1704x_devices_e device = static_cast<sfe_max1704x_devices_e>(PCONFIG(3));
      const int threshold = PCONFIG(1);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P115_data_struct(PCONFIG(0), device, threshold));
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P115_data) {
        success = P115_data->begin(); // Start the sensor
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      /*
      byte choice          = PCONFIG(0);
      int  optionValues[1] = { 0x36 };
      addFormSelectorI2C(F("plugin_115_i2c"), 1, optionValues, choice);
      */
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        unsigned int choice = PCONFIG(3);
        String options[4];
        options[0]          = F("MAX17043");
        options[1]          = F("MAX17044 (2S)"); // 2-cell version of the MAX17043 (full-scale range of 10V)
        options[2]          = F("MAX17048");
        options[3]          = F("MAX17049 (2S)"); // 2-cell version of the MAX17048
        int optionValues[4] = { 0, 1, 2, 3 };
        addFormSelector(F("Device"), F("plugin_115_device"), 4, options, optionValues, choice);
      }

      addFormNumericBox(F("Alert threshold"), F("plugin_115_threshold"), PCONFIG(1), 1, 30);
      addUnit(F("%"));
      addFormCheckBox(F("Send Event on Alert"), F("plugin_115_alertevent"), PCONFIG(2));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
//      PCONFIG(0) = getFormItemInt(F("plugin_115_i2c"));
      PCONFIG(1) = getFormItemInt(F("plugin_115_threshold"));
      PCONFIG(2) = isFormItemChecked(F("plugin_115_alertevent"));
      PCONFIG(3) = getFormItemInt(F("plugin_115_device"));

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P115_data) {
        UserVar[event->BaseVarIndex + 0] = P115_data->voltage;
        UserVar[event->BaseVarIndex + 1] = P115_data->soc;
        UserVar[event->BaseVarIndex + 2] = P115_data->alert;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(48);
          log  = F("MAX1704x : Voltage: ");
          log += P115_data->voltage;
          log += F(" SoC: ");
          log += P115_data->soc;
          log += F(" Alert: ");
          log += P115_data->alert;
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P115_data) {
        const String command = parseString(string, 1);

        if ((command == F("max1704xclearalert")))
        {
          P115_data->clearAlert();
          success = true;
        }
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P115_data) {
        if (P115_data->read(false)) {
          if (!P115_data->alert) {
            P115_data->alert = true;

            if (PCONFIG(2)) {
              // Need to send an event.
              if (Settings.UseRules) {
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

                if (validDeviceIndex(DeviceIndex)) {
                  LoadTaskSettings(event->TaskIndex);
                  String newEvent = getTaskDeviceName(event->TaskIndex);
                  newEvent += '#';
                  newEvent += F("AlertTriggered");
                  newEvent += ',';
                  newEvent += formatUserVarNoCheck(event, 0); // Voltage
                  newEvent += ',';
                  newEvent += formatUserVarNoCheck(event, 1); // State Of Charge
                  eventQueue.addMove(std::move(newEvent));
                }
              }
            }
          }
        }
        success = true;
      }
      break;
    }
  }    // switch
  return success;
}      // function

#endif // USES_P115
