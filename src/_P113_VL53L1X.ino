#include "_Plugin_Helper.h"
#ifdef USES_P113

// #######################################################################################################
// ########################### Plugin 113 VL53L1X I2C Ranging LIDAR      #################################
// #######################################################################################################

// Changelog:
// 2021-04-06, tonhuisman: Remove Interval optional attribute to avoid system overload, cleanup source
// 2021-04-05, tonhuisman: Add VL53L1X Time of Flight sensor to main repo (similar to but not compatible with VL53L0X)

// needs SparkFun_VL53L1X library from https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library

# include "src/PluginStructs/P113_data_struct.h"

# define PLUGIN_113
# define PLUGIN_ID_113         113
# define PLUGIN_NAME_113       "Distance - VL53L1X (400cm)"
# define PLUGIN_VALUENAME1_113 "Distance"
# define PLUGIN_VALUENAME2_113 "Ambient"


boolean Plugin_113(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_113;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_113);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_113));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_113));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x29, 0x30 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c"), 2, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsMode2[] = {
          F("100ms (Normal)"),
          F("20ms (Fastest)"),
          F("33ms (Fast)"),
          F("50ms"),
          F("200ms (Accurate)"),
          F("500ms"),
        };
        const int optionValuesMode2[] = { 100, 20, 33, 50, 200, 500 };
        addFormSelector(F("Timing"), F("timing"), 6, optionsMode2, optionValuesMode2, PCONFIG(1));
      }

      {
        const __FlashStringHelper *optionsMode3[] = {
          F("Normal (~130cm)"),
          F("Long (~400cm)"),
        };
        const int optionValuesMode3[2] = { 0, 1 };
        addFormSelector(F("Range"), F("range"), 2, optionsMode3, optionValuesMode3, PCONFIG(2));
      }
      addFormCheckBox(F("Send event when value unchanged"), F("notchanged"), PCONFIG(3) == 1);
      addFormNote(F("When checked, 'Trigger delta' setting is ignored!"));

      addFormNumericBox(F("Trigger delta"), F("delta"), PCONFIG(4), 0, 100);
      addUnit(F("0-100mm"));
      addFormNote(F("Minimal change in Distance to trigger an event."));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c"));
      PCONFIG(1) = getFormItemInt(F("timing"));
      PCONFIG(2) = getFormItemInt(F("range"));
      PCONFIG(3) = isFormItemChecked(F("notchanged")) ? 1 : 0;
      PCONFIG(4) = getFormItemInt(F("delta"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P113_data_struct(PCONFIG(0), PCONFIG(1), PCONFIG(2) == 1));
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P113_data) && P113_data->begin(); // Start the sensor
      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P113_data) {
        uint16_t dist      = P113_data->readDistance();
        uint16_t ambient   = P113_data->readAmbient();
        bool     triggered = (dist > UserVar[event->BaseVarIndex] + PCONFIG(4)) || (dist < UserVar[event->BaseVarIndex] - PCONFIG(4));

        if (P113_data->isReadSuccessful() && (triggered || (PCONFIG(3) == 1)) && (dist != 0xFFFF)) {
          UserVar[event->BaseVarIndex + 0] = dist;
          UserVar[event->BaseVarIndex + 1] = ambient;
          success                          = true;
        }
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P113_data) {
        if (P113_data->startRead()) {
          if (P113_data->readAvailable() && (Settings.TaskDeviceTimer[event->TaskIndex] == 0)) { // Trigger as soon as there's a valid
                                                                                                 // measurement and the time-out is set to 0
            Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P113
