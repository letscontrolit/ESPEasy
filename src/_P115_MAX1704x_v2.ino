/*
 * Plugin 115 MAX1704x I2C, Sparkfun Fuel Gauge Sensor
 *
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
# define PLUGIN_NAME_115   "Energy - Fuel Gauge MAX1704x"
# define PLUGIN_VALUENAME1_115 "Voltage" // Battery voltage
# define PLUGIN_VALUENAME2_115 "SOC"     // Battery state of charge in percentage
# define PLUGIN_VALUENAME3_115 "Alert"   // (0 or 1) Alert when the battery SoC gets too low
# define PLUGIN_VALUENAME4_115 "Rate"    // (MAX17048/49) Get rate of change per hour in %
# define PLUGIN_xxx_DEBUG  false         // set to true for extra log info in the debug

# define P115_I2CADDR         PCONFIG(0)
# define P115_THRESHOLD       PCONFIG(1)
# define P115_ALERTEVENT      PCONFIG(2)
# define P115_DEVICESELECTOR  PCONFIG(3)

boolean Plugin_115(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_115;
      dev.Type           = DEVICE_TYPE_I2C;                  // how the device is connected
      dev.VType          = Sensor_VType::SENSOR_TYPE_TRIPLE; // type of value the plugin will return
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.DecimalsOnly   = true;
      dev.PluginStats    = true;
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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_115));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x36);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x36;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_INIT:
    {
      const sfe_max1704x_devices_e device = static_cast<sfe_max1704x_devices_e>(P115_DEVICESELECTOR);
      const int threshold                 = P115_THRESHOLD;
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P115_data_struct(device, threshold));
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P115_data) && P115_data->begin(); // Start the sensor
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        unsigned int choice                   = P115_DEVICESELECTOR;
        const __FlashStringHelper *options[4] = {
          F("MAX17043"),
          F("MAX17044 (2S)"), // 2-cell version of the MAX17043 (full-scale range of 10V)
          F("MAX17048"),
          F("MAX17049 (2S)")  // 2-cell version of the MAX17048
        };
        const int optionValues[4] = {
          MAX1704X_MAX17043,
          MAX1704X_MAX17044,
          MAX1704X_MAX17048,
          MAX1704X_MAX17049 };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("Device"), F("device"), optionCount, options, optionValues, choice);
      }

      addFormNumericBox(F("Alert threshold"), F("threshold"), P115_THRESHOLD, 1, 32);
      addUnit('%');
      addFormCheckBox(F("Send Event on Alert"), F("alertevent"), P115_ALERTEVENT);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P115_THRESHOLD      = getFormItemInt(F("threshold"));
      P115_ALERTEVENT     = isFormItemChecked(F("alertevent"));
      P115_DEVICESELECTOR = getFormItemInt(F("device"));

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P115_data) && P115_data->initialized) {
        UserVar.setFloat(event->TaskIndex, 0, P115_data->voltage);
        UserVar.setFloat(event->TaskIndex, 1, P115_data->soc);
        UserVar.setFloat(event->TaskIndex, 2, P115_data->alert);
        UserVar.setFloat(event->TaskIndex, 3, P115_data->changeRate);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, strformat(F("MAX1704x : Voltage: %.2f SoC: %.2f Alert: %d Rate: %.2f"),
                                               P115_data->voltage, P115_data->soc, P115_data->alert, P115_data->changeRate));
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P115_data) && P115_data->initialized) {
        const String command = parseString(string, 1);

        if (equals(command, F("max1704xclearalert")))
        {
          P115_data->clearAlert();
          success = true;
        }
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P115_data_struct *P115_data = static_cast<P115_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P115_data) && P115_data->initialized) {
        if (P115_data->read(false)) {
          if (!P115_data->alert) {
            P115_data->alert = true;

            if (P115_ALERTEVENT) {
              // Need to send an event.
              if (Settings.UseRules) {
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

                if (validDeviceIndex(DeviceIndex)) {
                  String eventvalues = strformat(F("%s,%s"),
                                                 formatUserVarNoCheck(event, 0).c_str(),  // Voltage
                                                 formatUserVarNoCheck(event, 1).c_str()); // State Of Charge
                  eventQueue.add(event->TaskIndex, F("AlertTriggered"), eventvalues);
                }
              }
            }
          }
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P115
